// sole owner of all BLE UUIDs / device name
const DEVICE_NAME  = 'ESP32-Glove';
const SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NOTIFY_UUID  = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

// Packet layout (22 bytes, all int16_t little-endian):
//   bytes  0–9:  flex[5]   – baseline-subtracted ADC deltas, thumb→pinky
//   bytes 10–15: accel[3]  – ax, ay, az  (MPU6050 raw, ±2 g range)
//   bytes 16–21: gyro[3]   – gx, gy, gz  (MPU6050 raw, ±250 °/s range)
const PACKET_BYTES = 22;

export class BleGlove {
  constructor({ onData, onStatus }) {
    this._onData   = onData;
    this._onStatus = onStatus;
    this._device   = null;
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
      const buf = e.target.value;
      if (buf.byteLength < PACKET_BYTES) return;

      const v = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
      this._onData({
        flex:  Array.from({ length: 5 }, (_, i) => v.getInt16(i * 2,       true)),
        accel: Array.from({ length: 3 }, (_, i) => v.getInt16(10 + i * 2,  true)),
        gyro:  Array.from({ length: 3 }, (_, i) => v.getInt16(16 + i * 2,  true)),
      });
    });

    this._onStatus('connected');
  }

  disconnect() {
    if (this._device?.gatt.connected) {
      this._device.gatt.disconnect();
    }
  }
}
