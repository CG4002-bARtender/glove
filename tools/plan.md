# tools/visualizer — Three.js BLE Visualizer

## Purpose

Browser-based tool that connects directly to an ESP32 glove device over BLE (Web Bluetooth API),
receives sensor data via a notify characteristic, and displays it. Built with Three.js as the
foundation for a larger 3D hand-visualisation application; this demo proves the pipeline end-to-end
with a rotating cube and a scrolling data log.

No firmware internals are referenced here. The visualizer depends only on the BLE contract below.

---

## BLE Contract

| Property         | Value                                   |
|------------------|-----------------------------------------|
| Device name      | `ESP32-Glove`                           |
| Service UUID     | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| Notify char      | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` (device → browser) |
| Write char       | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` (browser → device) |
| Encoding         | UTF-8                                   |

### Demo packet format
```
ping <uint32>
```
Example: `ping 42`  — one packet per second.

### Full packet format (future)
Newline-terminated JSON:
```json
{"seq":42,"imu":{"ax":-1234,"ay":567,"az":9800,"gx":100,"gy":-50,"gz":30},"flex":[100,200,150,180,90]}
```

| Field          | Type   | Notes                          |
|----------------|--------|--------------------------------|
| `seq`          | uint32 | Packet counter                 |
| `imu.ax/ay/az` | int16  | Raw accelerometer counts       |
| `imu.gx/gy/gz` | int16  | Raw gyroscope counts           |
| `flex[0–4]`    | int    | Baseline-corrected ADC (0–4095)|

---

## Tech Stack

| Tool              | Role                                           |
|-------------------|------------------------------------------------|
| Vite              | Dev server + ES module bundler                 |
| Three.js          | 3D rendering — foundation for 3D hand model    |
| Web Bluetooth API | Browser-native BLE, no Python backend needed   |
| Vanilla JS        | UI overlay (status badge, data log)            |

**Browser:** Chrome or Edge only (Web Bluetooth not supported in Firefox/Safari).

---

## File Structure

```
tools/visualizer/
  package.json          npm project (three + vite)
  index.html            entry point — canvas + overlay div
  src/
    ble.js              sole owner of all BLE UUIDs / device name
    ui.js               DOM overlay: status badge, connect button, log panel
    main.js             Three.js scene + animation loop + wiring
```

---

## Architecture

```
main.js
  ├── imports BleGlove from ble.js
  │     └── Web Bluetooth API (browser built-in)
  ├── imports initUI from ui.js
  │     └── DOM overlay elements
  └── Three.js scene (WebGLRenderer, PerspectiveCamera, BoxGeometry)
```

Data flow on notification:
```
ESP32 → BLE notify → ble.js onData callback → ui.logMessage(string)
```

`ble.js` has zero knowledge of Three.js or the DOM.
`ui.js` has zero knowledge of BLE or Three.js.
`main.js` is the only file that imports from both.

---

## Running

```bash
cd tools/visualizer
npm install
npm run dev          # opens http://localhost:5173 in your terminal
```

Open in **Chrome or Edge**. The Three.js cube renders without any device connected.
Click **Connect**, select `ESP32-Glove` in the browser picker, and watch packets appear in the log.
