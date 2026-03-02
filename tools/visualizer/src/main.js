import * as THREE from 'three';
import { BleGlove } from './ble.js';
import { initUI } from './ui.js';
import { ImuTracker } from './imu.js';

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

scene.add(new THREE.AmbientLight(0xffffff, 0.6));
const dirLight = new THREE.DirectionalLight(0xffffff, 1.5);
dirLight.position.set(3, 5, 4);
scene.add(dirLight);

// ── Hand model ────────────────────────────────────────────────────────────
const palmMat   = new THREE.MeshStandardMaterial({ color: 0x2563eb, roughness: 0.4, metalness: 0.1 });
const fingerMat = new THREE.MeshStandardMaterial({ color: 0x60a5fa, roughness: 0.3, metalness: 0.1 });
const edgeMat   = new THREE.MeshBasicMaterial({ color: 0x1e3a8a, wireframe: true });

const palmGeo  = new THREE.BoxGeometry(1.15, 0.9, 0.1);
const handGroup = new THREE.Group();
handGroup.add(new THREE.Mesh(palmGeo, palmMat));
handGroup.add(new THREE.Mesh(palmGeo, edgeMat));

// Five fingers: thumb (index 0, left) → pinky (index 4, right)
const FINGER_X = [-0.44, -0.22, 0.0, 0.22, 0.44];
const fingerPivots = FINGER_X.map((x) => {
  const pivot = new THREE.Group();
  pivot.position.set(x, 0.45, 0);

  const geo = new THREE.BoxGeometry(0.13, 0.48, 0.09);
  const mesh = new THREE.Mesh(geo, fingerMat);
  mesh.position.y = 0.24;
  pivot.add(mesh);
  const edgeMesh = new THREE.Mesh(geo, edgeMat);
  edgeMesh.position.y = 0.24;
  pivot.add(edgeMesh);

  handGroup.add(pivot);
  return pivot;
});

// handPivot rotates the model -90° around X so it starts flat (fingers into
// screen, palm down) instead of the geometry's default "fingers up" pose.
// The IMU quaternion is applied to handGroup (the child) so its rotations are
// expressed in this corrected frame, where X=pitch, Y=roll, Z=yaw.
const handPivot = new THREE.Group();
handPivot.rotation.x = -Math.PI / 2;
handPivot.add(handGroup);
scene.add(handPivot);

// Fixed world-axes reference (stays in place while hand rotates)
// Red = X, Green = Y, Blue = Z
scene.add(new THREE.AxesHelper(1.5));

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

// ── IMU tracker ───────────────────────────────────────────────────────────
const imu = new ImuTracker();

// ── Sample renderer ───────────────────────────────────────────────────────
function applySample(pkt) {
  // Fingers (local rotation relative to handGroup)
  for (let i = 0; i < 5; i++) {
    const n = pkt.flex[i];  // 0 = straight, 1 = flexed
    fingerPivots[i].rotation.x = n * (Math.PI / 2);
    flexFills[i].style.height  = `${n * 100}%`;
  }

  // IMU orientation (rotates the whole handGroup)
  imu.update(pkt.accel, pkt.gyro);
  imu.applyTo(handGroup);

  // Update IMU readout
  const [rx, ry, rz] = imu.degrees;
  document.getElementById('imu-rx').textContent = rx.toFixed(1) + '°';
  document.getElementById('imu-ry').textContent = ry.toFixed(1) + '°';
  document.getElementById('imu-rz').textContent = rz.toFixed(1) + '°';
}

// ── Gesture replay ────────────────────────────────────────────────────────
const replayBtn  = document.getElementById('replay-btn');
const replayInfo = document.getElementById('replay-info');

let replayData   = null;
let replayIdx    = 0;
let replayNextMs = 0;
let replayActive = false;

const SAMPLE_INTERVAL_MS = 1000 / 30;

replayBtn.disabled = true;

replayBtn.addEventListener('click', () => {
  if (!replayData) return;
  replayIdx    = 0;
  replayNextMs = performance.now();
  replayActive = true;
  replayBtn.disabled = true;
  imu.bootstrapFromAccel(replayData[0].accel);
});

// ── Gesture handler ───────────────────────────────────────────────────────
function onGesture(samples) {
  console.log(`[Gesture] received ${samples.length} samples`);
  replayData   = samples;
  replayActive = false;
  replayBtn.disabled = false;
  replayInfo.textContent = '';
}

// ── Render loop ───────────────────────────────────────────────────────────
(function animate() {
  requestAnimationFrame(animate);

  if (replayActive && replayData) {
    const now = performance.now();
    while (replayIdx < replayData.length && now >= replayNextMs) {
      applySample(replayData[replayIdx++]);
      replayNextMs += SAMPLE_INTERVAL_MS;
    }
    replayInfo.textContent = `▶ ${replayIdx} / ${replayData.length}`;

    if (replayIdx >= replayData.length) {
      replayActive = false;
      replayBtn.disabled = false;
      replayInfo.textContent = `${replayData.length} frames`;
      for (let i = 0; i < 5; i++) {
        fingerPivots[i].rotation.x = 0;
        flexFills[i].style.height  = '0%';
      }
      handGroup.quaternion.identity();
    }
  }

  renderer.render(scene, camera);
})();

window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

// ── BLE debug panel ───────────────────────────────────────────────────────
const dp = {
  total:    document.getElementById('dp-total'),
  size:     document.getElementById('dp-size'),
  start:    document.getElementById('dp-start'),
  sample:   document.getElementById('dp-sample'),
  end:      document.getElementById('dp-end'),
  unknown:  document.getElementById('dp-unknown'),
  lasttype: document.getElementById('dp-lasttype'),
};
const dpCount = { total: 0, start: 0, sample: 0, end: 0, unknown: 0 };

function onRaw(byteLen, info) {
  dpCount.total++;
  dp.total.textContent = dpCount.total;
  dp.size.textContent  = `${byteLen}B`;

  switch (info.type) {
    case 'start':
      dpCount.start++;
      dp.start.textContent    = dpCount.start;
      dp.lasttype.textContent = `START (exp ${info.expected})`;
      dp.lasttype.className   = 'dp-val dp-ok';
      break;
    case 'sample':
      dpCount.sample++;
      dp.sample.textContent   = dpCount.sample;
      dp.lasttype.textContent = `SAMPLE #${info.count}`;
      dp.lasttype.className   = 'dp-val dp-ok';
      break;
    case 'end':
      dpCount.end++;
      dp.end.textContent      = dpCount.end;
      dp.lasttype.textContent = `END (${info.collected} smp)`;
      dp.lasttype.className   = 'dp-val dp-ok';
      break;
    default:
      dpCount.unknown++;
      dp.unknown.textContent  = dpCount.unknown;
      dp.lasttype.textContent = `0x${info.byte.toString(16).padStart(2,'0')} (unknown)`;
      dp.lasttype.className   = 'dp-val dp-warn';
  }
}

// ── BLE + UI ──────────────────────────────────────────────────────────────
const { setStatus } = initUI({ onConnect: handleConnect });

const glove = new BleGlove({ onGesture, onRaw, onStatus: setStatus });

async function handleConnect() {
  try {
    await glove.connect();
  } catch (err) {
    setStatus('error');
  }
}
