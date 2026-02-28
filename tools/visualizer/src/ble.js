// sole owner of all BLE UUIDs / device name
const DEVICE_NAME = 'ESP32-Glove';
const SERVICE_UUID  = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NOTIFY_UUID   = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
const WRITE_UUID    = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';

const decoder = new TextDecoder();

export class BleGlove {
  constructor({ onData, onStatus }) {
    this._onData   = onData;
    this._onStatus = onStatus;
    this._device   = null;
    this._server   = null;
    this._writeChr = null;
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

    this._server = await this._device.gatt.connect();
    const service = await this._server.getPrimaryService(SERVICE_UUID);

    // subscribe to notifications
    const notifyChr = await service.getCharacteristic(NOTIFY_UUID);
    await notifyChr.startNotifications();
    notifyChr.addEventListener('characteristicvaluechanged', (e) => {
      const text = decoder.decode(e.target.value).trim();
      this._onData(text);
    });

    // keep write characteristic for optional bidirectional use
    this._writeChr = await service.getCharacteristic(WRITE_UUID);

    this._onStatus('connected');
  }

  async write(text) {
    if (!this._writeChr) return;
    const encoded = new TextEncoder().encode(text);
    await this._writeChr.writeValueWithoutResponse(encoded);
  }

  disconnect() {
    if (this._device?.gatt.connected) {
      this._device.gatt.disconnect();
    }
  }
}
