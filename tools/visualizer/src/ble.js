// Sole owner of all BLE UUIDs / device name and packet parsing.
const DEVICE_NAME  = 'ESP32-Glove';
const SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NOTIFY_UUID  = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

// ── Gesture framing protocol ──────────────────────────────────────────────
// Each BLE notify carries one of three packet types (first byte = type):
//   0x01  GESTURE_START   [count_lo][count_hi]              3 bytes total
//   0x02  GESTURE_SAMPLE  [n][n × 13-byte payload]          2+n*13 bytes
//   0x03  GESTURE_END                                        1 byte  total
//
// 13-byte sample layout:
//   byte  0:    flex_bits  — 5-bit bitmask, bit i = finger i flexed (thumb=0, pinky=4)
//   bytes 1–6:  accel[3]   — ax, ay, az  (int16_t, little-endian)
//   bytes 7–12: gyro[3]    — gx, gy, gz  (int16_t, little-endian)
const PKT_GESTURE_START  = 0x01;
const PKT_GESTURE_SAMPLE = 0x02;
const PKT_GESTURE_END    = 0x03;
const SAMPLE_BYTES = 13;

function parseSample(view, offset) {
  const flexBits = view.getUint8(offset);
  return {
    flex:  Array.from({ length: 5 }, (_, i) => (flexBits >> (4 - i)) & 1),
    accel: Array.from({ length: 3 }, (_, i) => view.getInt16(offset + 1 + i * 2, true)),
    gyro:  Array.from({ length: 3 }, (_, i) => view.getInt16(offset + 7 + i * 2, true)),
  };
}

// ms to wait after last SAMPLE before auto-completing a gesture with no END packet
const GESTURE_END_TIMEOUT_MS = 500;

export class BleGlove {
  constructor({ onGesture, onStatus, onRaw }) {
    this._onGesture = onGesture;
    this._onStatus  = onStatus;
    this._onRaw     = onRaw || null;
    this._device    = null;

    // Accumulator for in-flight gesture window
    this._gestureAcc      = [];
    this._gestureExpected = 0;
    this._endTimer        = null;
  }

  async connect() {
    this._onStatus('connecting');

    this._device = await navigator.bluetooth.requestDevice({
      filters: [{ name: DEVICE_NAME }],
      optionalServices: [SERVICE_UUID],
    });

    this._device.addEventListener('gattserverdisconnected', () => {
      this._onStatus('disconnected');
    });

    const server  = await this._device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    const chr     = await service.getCharacteristic(NOTIFY_UUID);
    await chr.startNotifications();

    chr.addEventListener('characteristicvaluechanged', (e) => {
      this._handleNotify(e.target.value);
    });

    this._onStatus('connected');
  }

  disconnect() {
    if (this._device?.gatt.connected) {
      this._device.gatt.disconnect();
    }
  }

  _flushGesture(reason) {
    this._clearEndTimer();
    if (this._gestureAcc.length > 0) {
      console.log(`[BLE] Gesture complete (${reason})  samples=${this._gestureAcc.length}`);
      this._onGesture(this._gestureAcc);
      this._gestureAcc      = [];
      this._gestureExpected = 0;
    }
  }

  _armEndTimer() {
    this._clearEndTimer();
    this._endTimer = setTimeout(() => {
      console.warn(`[BLE] No GESTURE_END received — auto-completing after ${GESTURE_END_TIMEOUT_MS} ms`);
      this._flushGesture('timeout');
    }, GESTURE_END_TIMEOUT_MS);
  }

  _clearEndTimer() {
    if (this._endTimer !== null) {
      clearTimeout(this._endTimer);
      this._endTimer = null;
    }
  }

  _handleNotify(buf) {
    if (buf.byteLength === 0) return;
    const v    = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
    const type = v.getUint8(0);

    // Log every raw packet for debugging
    const hexHead = Array.from(
      { length: Math.min(buf.byteLength, 8) },
      (_, i) => v.getUint8(i).toString(16).padStart(2, '0'),
    ).join(' ');
    console.log(`[BLE raw] ${buf.byteLength}B  type=0x${type.toString(16).padStart(2,'0')}  [${hexHead}${buf.byteLength > 8 ? ' ...' : ''}]`);

    let info;
    switch (type) {
      case PKT_GESTURE_START:
        if (buf.byteLength < 3) { console.warn('[BLE] GESTURE_START too short:', buf.byteLength); return; }
        this._clearEndTimer();
        this._gestureExpected = v.getUint16(1, true);
        this._gestureAcc      = [];
        info = { type: 'start', expected: this._gestureExpected };
        console.log(`[BLE] GESTURE_START  expected=${this._gestureExpected} samples`);
        break;

      case PKT_GESTURE_SAMPLE: {
        // Layout: [0x02][n][n × SAMPLE_BYTES]
        if (buf.byteLength < 2) { console.warn('[BLE] GESTURE_SAMPLE too short:', buf.byteLength); return; }
        const n = v.getUint8(1);
        if (buf.byteLength < 2 + n * SAMPLE_BYTES) { console.warn('[BLE] GESTURE_SAMPLE truncated, expected', 2 + n * SAMPLE_BYTES, 'got', buf.byteLength); return; }
        for (let i = 0; i < n; i++) this._gestureAcc.push(parseSample(v, 2 + i * SAMPLE_BYTES));
        info = { type: 'sample', count: this._gestureAcc.length };
        console.log(`[BLE] GESTURE_SAMPLE  batch=${n}  total=${this._gestureAcc.length}`);
        this._armEndTimer();  // reset countdown on every batch
        break;
      }

      case PKT_GESTURE_END:
        info = { type: 'end', collected: this._gestureAcc.length };
        console.log(`[BLE] GESTURE_END  collected=${this._gestureAcc.length} samples`);
        this._flushGesture('END packet');
        break;

      default:
        info = { type: 'unknown', byte: type };
        console.warn(`[BLE] Unknown packet type: 0x${type.toString(16)}  (${buf.byteLength}B) — firmware may be sending raw (unframed) packets`);
    }

    if (this._onRaw) this._onRaw(buf.byteLength, info);
  }
}
