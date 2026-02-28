import * as THREE from 'three';
import { BleGlove } from './ble.js';
import { initUI } from './ui.js';

// --- UI ---
const { setStatus, logMessage } = initUI({ onConnect: handleConnect });

// --- Three.js scene ---
const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
renderer.setPixelRatio(window.devicePixelRatio);
renderer.setSize(window.innerWidth, window.innerHeight);
document.getElementById('canvas-container').appendChild(renderer.domElement);

const scene  = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 100);
camera.position.set(0, 0, 3);

// Cube
const geometry = new THREE.BoxGeometry(1, 1, 1);
const material = new THREE.MeshStandardMaterial({
  color: 0x4f9cf0,
  roughness: 0.4,
  metalness: 0.3,
});
const cube = new THREE.Mesh(geometry, material);
scene.add(cube);

// Wireframe overlay
const wireMat = new THREE.MeshBasicMaterial({ color: 0x93c5fd, wireframe: true });
const wire = new THREE.Mesh(geometry, wireMat);
scene.add(wire);

// Lighting
scene.add(new THREE.AmbientLight(0xffffff, 0.5));
const dirLight = new THREE.DirectionalLight(0xffffff, 1.5);
dirLight.position.set(3, 5, 4);
scene.add(dirLight);

// Animation loop
function animate() {
  requestAnimationFrame(animate);
  cube.rotation.x += 0.005;
  cube.rotation.y += 0.008;
  wire.rotation.copy(cube.rotation);
  renderer.render(scene, camera);
}
animate();

window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

// --- BLE ---
const glove = new BleGlove({
  onData:   (msg) => logMessage(msg),
  onStatus: (s)   => {
    setStatus(s);
    if (s === 'disconnected') logMessage('Device disconnected.', 'info');
    if (s === 'connected')    logMessage('Connected to ESP32-Glove.', 'info');
  },
});

async function handleConnect() {
  try {
    await glove.connect();
  } catch (err) {
    setStatus('error');
    logMessage(`Connection failed: ${err.message}`, 'error');
  }
}
