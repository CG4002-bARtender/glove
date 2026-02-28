// DOM overlay: status badge + connect button
const dot  = document.getElementById('status-dot');
const text = document.getElementById('status-text');
const btn  = document.getElementById('connect-btn');

const STATUS_LABELS = {
  disconnected: 'Disconnected',
  connecting:   'Connecting…',
  connected:    'Connected',
  error:        'Error',
};

export function initUI({ onConnect }) {
  btn.addEventListener('click', onConnect);
  return { setStatus };
}

function setStatus(state) {
  dot.className = '';
  if (state === 'connected')  dot.classList.add('connected');
  if (state === 'connecting') dot.classList.add('connecting');
  if (state === 'error')      dot.classList.add('error');

  text.textContent = STATUS_LABELS[state] ?? state;
  btn.disabled     = state === 'connecting' || state === 'connected';
  btn.textContent  = state === 'connected' ? 'Connected' : 'Connect';
}
