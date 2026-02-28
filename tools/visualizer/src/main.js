import * as THREE from 'three';
import { BleGlove } from './ble.js';
import { initUI } from './ui.js';

// ── Sensor scaling constants ──────────────────────────────────────────────
const ACCEL_SCALE = 1.0 / 16384.0;            // g per LSB  (MPU6050 ±2 g)
const GYRO_SCALE  = (Math.PI / 180) / 131.0;  // rad/s per LSB (±250 °/s)
const FLEX_MAX    = 1500;                      // raw ADC units → full bend
const CF_ALPHA    = 0.96;                      // complementary filter: gyro weight

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
  pivot.add(Object.assign(new THREE.Mesh(geo, edgeMat), { position: mesh.position.clone() }));

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

// ── Orientation state (complementary filter) ──────────────────────────────
const ori = { pitch: 0, roll: 0 };
let lastTs = performance.now();

// ── Packet handler ────────────────────────────────────────────────────────
function onPacket(pkt) {
  const now = performance.now();
  const dt  = Math.min((now - lastTs) / 1000, 0.1); // cap at 100 ms
  lastTs = now;

  // Accelerometer → normalised gravity vector → accel-derived pitch/roll
  const ax  = pkt.accel[0] * ACCEL_SCALE;
  const ay  = pkt.accel[1] * ACCEL_SCALE;
  const az  = pkt.accel[2] * ACCEL_SCALE;
  const mag = Math.sqrt(ax * ax + ay * ay + az * az) || 1;

  const aPitch = Math.atan2(ay / mag, az / mag);
  const aRoll  = Math.atan2(-ax / mag, az / mag);

  // Gyroscope → angular velocity (rad/s)
  const gx = pkt.gyro[0] * GYRO_SCALE;
  const gy = pkt.gyro[1] * GYRO_SCALE;

  // Complementary filter: trust gyro short-term, correct drift with accel
  ori.pitch = CF_ALPHA * (ori.pitch + gx * dt) + (1 - CF_ALPHA) * aPitch;
  ori.roll  = CF_ALPHA * (ori.roll  + gy * dt) + (1 - CF_ALPHA) * aRoll;

  handGroup.rotation.x = ori.pitch;
  handGroup.rotation.z = ori.roll;

  // Flex: update finger pivots + DOM bars
  for (let i = 0; i < 5; i++) {
    const n = Math.max(0, Math.min(pkt.flex[i], FLEX_MAX)) / FLEX_MAX;
    fingerPivots[i].rotation.x  = n * (Math.PI / 2);   // curl toward palm
    flexFills[i].style.height   = `${(n * 100).toFixed(1)}%`;
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
  onStatus: (s) => setStatus(s),
});

async function handleConnect() {
  try {
    await glove.connect();
  } catch (err) {
    setStatus('error');
  }
}
