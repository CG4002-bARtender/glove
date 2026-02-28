import * as THREE from 'three';
import { BleGlove } from './ble.js';
import { initUI } from './ui.js';

// ── Sensor scaling constants ──────────────────────────────────────────────
const ACCEL_SCALE = 1.0 / 16384.0;            // g per LSB  (MPU6050 ±2 g)
const GYRO_SCALE  = (Math.PI / 180) / 131.0;  // rad/s per LSB (±250 °/s)
const FLEX_MAX    = 1500;                      // raw ADC units → full bend

// Hardcoded IMU mounting correction.
// Derived from observed axis behaviour:
//   raise hand (pitch) → was +Y, should be −X  →  old_Y maps to −new_X
//   roll right         → was +X, should be −Z  →  old_X maps to −new_Z
//   yaw left           → was +Z, should be +Y  →  old_Z maps to +new_Y
// Rotation matrix [[0,−1,0],[0,0,1],[−1,0,0]] → quaternion (w=0.5, x=−0.5, y=0.5, z=0.5)
const MOUNT_CORR     = new THREE.Quaternion(-0.5, 0.5, 0.5, 0.5);  // x,y,z,w
const MOUNT_CORR_INV = MOUNT_CORR.clone().invert();

function correctedQuat() {
  const q = madgwick.quaternion();
  return MOUNT_CORR.clone().multiply(q).multiply(MOUNT_CORR_INV);
}

// ── Three.js setup ────────────────────────────────────────────────────────
const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
renderer.setPixelRatio(window.devicePixelRatio);
renderer.setSize(window.innerWidth, window.innerHeight);
document.getElementById('canvas-container').appendChild(renderer.domElement);

const scene  = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
  55, window.innerWidth / window.innerHeight, 0.1, 100,
);
camera.position.set(0, 0.5, 4.5);
camera.lookAt(0, 0.3, 0);

// Lighting
scene.add(new THREE.AmbientLight(0xffffff, 0.6));
const dirLight = new THREE.DirectionalLight(0xffffff, 1.5);
dirLight.position.set(3, 5, 4);
scene.add(dirLight);

// Ground grid (orientation reference)
const grid = new THREE.GridHelper(8, 16, 0x222244, 0x111133);
grid.position.y = -1.5;
scene.add(grid);

// ── Hand model ────────────────────────────────────────────────────────────
const palmMat   = new THREE.MeshStandardMaterial({ color: 0x2563eb, roughness: 0.4, metalness: 0.1 });
const fingerMat = new THREE.MeshStandardMaterial({ color: 0x60a5fa, roughness: 0.3, metalness: 0.1 });
const edgeMat   = new THREE.MeshBasicMaterial({ color: 0x1e3a8a, wireframe: true });

// Palm (faces camera along Z)
const palmGeo  = new THREE.BoxGeometry(1.15, 0.9, 0.1);
const handGroup = new THREE.Group();
handGroup.add(new THREE.Mesh(palmGeo, palmMat));
handGroup.add(new THREE.Mesh(palmGeo, edgeMat));

// Five fingers: thumb (index 0, left) → pinky (index 4, right)
// Each pivot sits at the top edge of the palm; finger mesh extends upward from pivot.
const FINGER_X = [-0.44, -0.22, 0.0, 0.22, 0.44];
const fingerPivots = FINGER_X.map((x) => {
  const pivot = new THREE.Group();
  pivot.position.set(x, 0.45, 0);   // top edge of palm

  const geo = new THREE.BoxGeometry(0.13, 0.48, 0.09);
  const mesh = new THREE.Mesh(geo, fingerMat);
  mesh.position.y = 0.24;            // pivot at base, mesh centre above it
  pivot.add(mesh);
  const edgeMesh = new THREE.Mesh(geo, edgeMat);
  edgeMesh.position.y = 0.24;
  pivot.add(edgeMesh);

  handGroup.add(pivot);
  return pivot;
});

scene.add(handGroup);

// ── Flex bars (DOM) ───────────────────────────────────────────────────────
const FINGER_LABELS = ['T', 'I', 'M', 'R', 'P'];
const flexFills = [];

const flexPanel = document.getElementById('flex-panel');
FINGER_LABELS.forEach((lbl) => {
  const col   = document.createElement('div');
  col.className = 'f-col';

  const track = document.createElement('div');
  track.className = 'f-track';

  const fill  = document.createElement('div');
  fill.className = 'f-fill';
  fill.style.height = '0%';

  const label = document.createElement('div');
  label.className = 'f-label';
  label.textContent = lbl;

  track.appendChild(fill);
  col.appendChild(track);
  col.appendChild(label);
  flexPanel.appendChild(col);
  flexFills.push(fill);
});

// ── Madgwick AHRS filter ──────────────────────────────────────────────────
// Fuses accel + all 3 gyro axes into a full quaternion orientation.
// beta: algorithm gain — higher = trust accel more (stable but sluggish),
//                        lower  = trust gyro more  (responsive but drifty).
class Madgwick {
  constructor(beta = 0.1) {
    this.beta = beta;
    this.q    = [1, 0, 0, 0];  // [w, x, y, z]
  }

  update(ax, ay, az, gx, gy, gz, dt) {
    let [q0, q1, q2, q3] = this.q;

    // Normalize accelerometer; bail if zero (free-fall / sensor fault)
    const aNorm = Math.sqrt(ax*ax + ay*ay + az*az);
    if (aNorm === 0) return;
    ax /= aNorm; ay /= aNorm; az /= aNorm;

    // Objective function: difference between predicted and measured gravity
    const f1 = 2*(q1*q3 - q0*q2) - ax;
    const f2 = 2*(q0*q1 + q2*q3) - ay;
    const f3 = 2*(0.5 - q1*q1 - q2*q2) - az;

    // Jacobian (J^T layout)
    const j11 = -2*q2; const j12 =  2*q3; const j13 = -2*q0; const j14 =  2*q1;
    const j21 =  2*q1; const j22 =  2*q0; const j23 =  2*q3; const j24 =  2*q2;
    const j31 =  0;    const j32 = -4*q1; const j33 = -4*q2; const j34 =  0;

    // Gradient (J^T · f), normalised
    let s0 = j11*f1 + j21*f2 + j31*f3;
    let s1 = j12*f1 + j22*f2 + j32*f3;
    let s2 = j13*f1 + j23*f2 + j33*f3;
    let s3 = j14*f1 + j24*f2 + j34*f3;
    const sNorm = Math.sqrt(s0*s0 + s1*s1 + s2*s2 + s3*s3);
    s0 /= sNorm; s1 /= sNorm; s2 /= sNorm; s3 /= sNorm;

    // Quaternion derivative from gyroscope
    const qd0 = 0.5 * (-q1*gx - q2*gy - q3*gz);
    const qd1 = 0.5 * ( q0*gx + q2*gz - q3*gy);
    const qd2 = 0.5 * ( q0*gy - q1*gz + q3*gx);
    const qd3 = 0.5 * ( q0*gz + q1*gy - q2*gx);

    // Integrate: gyro rate corrected by gradient-descent feedback
    q0 += (qd0 - this.beta * s0) * dt;
    q1 += (qd1 - this.beta * s1) * dt;
    q2 += (qd2 - this.beta * s2) * dt;
    q3 += (qd3 - this.beta * s3) * dt;

    const qNorm = Math.sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    this.q = [q0/qNorm, q1/qNorm, q2/qNorm, q3/qNorm];
  }

  // THREE.Quaternion uses (x, y, z, w) order
  quaternion() {
    const [w, x, y, z] = this.q;
    return new THREE.Quaternion(x, y, z, w);
  }

  reset() { this.q = [1, 0, 0, 0]; }
}

const madgwick = new Madgwick(0.1);

// ── Reference frame ───────────────────────────────────────────────────────
const refQuat = new THREE.Quaternion();
let   hasRef  = false;
let   lastTs  = performance.now();

// ── Orient button ─────────────────────────────────────────────────────────
const orientBtn = document.getElementById('orient-btn');
orientBtn.disabled = true;

orientBtn.addEventListener('click', () => {
  refQuat.copy(correctedQuat());
  hasRef = true;
  orientBtn.textContent = 'Re-orient';
});

// ── Packet handler ────────────────────────────────────────────────────────
function onPacket(pkt) {
  const now = performance.now();
  const dt  = Math.min((now - lastTs) / 1000, 0.1);
  lastTs = now;

  const ax = pkt.accel[0] * ACCEL_SCALE;
  const ay = pkt.accel[1] * ACCEL_SCALE;
  const az = pkt.accel[2] * ACCEL_SCALE;
  const gx = pkt.gyro[0]  * GYRO_SCALE;
  const gy = pkt.gyro[1]  * GYRO_SCALE;
  const gz = pkt.gyro[2]  * GYRO_SCALE;

  madgwick.update(ax, ay, az, gx, gy, gz, dt);

  const currentQuat = correctedQuat();
  if (hasRef) {
    handGroup.quaternion.copy(refQuat.clone().invert().multiply(currentQuat));
  } else {
    handGroup.quaternion.copy(currentQuat);
  }

  // Flex: update finger pivots + DOM bars
  for (let i = 0; i < 5; i++) {
    const n = Math.max(0, Math.min(pkt.flex[i], FLEX_MAX)) / FLEX_MAX;
    fingerPivots[i].rotation.x = n * (Math.PI / 2);
    flexFills[i].style.height  = `${(n * 100).toFixed(1)}%`;
  }
}

// ── Render loop ───────────────────────────────────────────────────────────
(function animate() {
  requestAnimationFrame(animate);
  renderer.render(scene, camera);
})();

window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

// ── BLE + UI ──────────────────────────────────────────────────────────────
const { setStatus } = initUI({ onConnect: handleConnect });

const glove = new BleGlove({
  onData:   onPacket,
  onStatus: (s) => {
    setStatus(s);
    orientBtn.disabled = s !== 'connected';
    if (s === 'disconnected') {
      hasRef = false;
      madgwick.reset();
      orientBtn.textContent = 'Orient';
    }
  },
});

async function handleConnect() {
  try {
    await glove.connect();
  } catch (err) {
    setStatus('error');
  }
}
