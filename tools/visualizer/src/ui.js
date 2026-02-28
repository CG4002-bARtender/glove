// DOM overlay: status badge, connect button, log panel
// No knowledge of BLE or Three.js

const MAX_LOG_LINES = 80;

const dot    = document.getElementById('status-dot');
const text   = document.getElementById('status-text');
const btn    = document.getElementById('connect-btn');
const logEl  = document.getElementById('log-panel');

const STATUS_LABELS = {
  disconnected: 'Disconnected',
  connecting:   'Connecting…',
  connected:    'Connected',
  error:        'Error',
};

export function initUI({ onConnect }) {
  btn.addEventListener('click', onConnect);
  return { setStatus, logMessage };
}

function setStatus(state) {
  dot.className = '';
  if (state === 'connected')   dot.classList.add('connected');
  if (state === 'connecting')  dot.classList.add('connecting');
  if (state === 'error')       dot.classList.add('error');

  text.textContent = STATUS_LABELS[state] ?? state;
  btn.disabled = state === 'connecting' || state === 'connected';
  btn.textContent = state === 'connected' ? 'Connected' : 'Connect';
}

function logMessage(msg, type = 'log') {
  const line = document.createElement('div');
  line.className = 'log-line' + (type !== 'log' ? ` ${type}` : '');
  line.textContent = `[${timestamp()}] ${msg}`;
  logEl.appendChild(line);

  // trim old lines
  while (logEl.children.length > MAX_LOG_LINES) {
    logEl.removeChild(logEl.firstChild);
  }

  logEl.scrollTop = logEl.scrollHeight;
}

function timestamp() {
  const now = new Date();
  return `${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;
}

function pad(n) { return String(n).padStart(2, '0'); }
