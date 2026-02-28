// sole owner of all BLE UUIDs / device name
const DEVICE_NAME  = 'ESP32-Glove';
const SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NOTIFY_UUID  = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

const decoder = new TextDecoder();

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

    const notifyChr = await service.getCharacteristic(NOTIFY_UUID);
    await notifyChr.startNotifications();
    notifyChr.addEventListener('characteristicvaluechanged', (e) => {
      this._onData(decoder.decode(e.target.value).trim());
    });

    this._onStatus('connected');
  }

  disconnect() {
    if (this._device?.gatt.connected) {
      this._device.gatt.disconnect();
    }
  }
}
