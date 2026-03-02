import * as THREE from 'three';

// MPU6050 default sensitivity
const ACCEL_SCALE = 1 / 16384;              // raw int16 → g       (±2g)
const GYRO_SCALE  = (Math.PI / 180) / 131;  // raw int16 → rad/s   (±250°/s)
const DT          = 1 / 30;                 // seconds per sample (30 Hz)

// Madgwick algorithm gain (β).
// Controls how aggressively accel corrects gyro drift.
// Lower (0.01–0.05) = smoother, more drift.  Higher (0.1–0.3) = faster correction, more accel noise.
const BETA = 0.1;

/**
 * Madgwick AHRS filter (6-DOF, no magnetometer).
 *
 * Physical → IMU axis mapping (confirmed from glove mounting):
 *   gx → roll   (+gx = thumb-down)
 *   gy → pitch  (+gy = fingers-up)
 *   gz → yaw    (+gz = CCW from above)
 *
 * Both gyro and accel are remapped to the child (visual) frame before
 * processing. The child frame has the -PI/2 X base offset baked into the
 * handPivot parent group, so at rest (hand flat) the quaternion is identity.
 *
 * Child frame axes:
 *   child X (pitch) ← gy_imu / ay_imu
 *   child Y (roll)  ← -gx_imu / -ax_imu
 *   child Z (up)    ← gz_imu / az_imu
 *
 * At rest (hand flat, IMU Z pointing up): accel = (0, 0, +1) in child frame.
 * This is the gravity reference the Madgwick filter corrects toward.
 *
 * Algorithm (Madgwick 2010, eq. 33, 6-DOF):
 *   qdot = 0.5 * q ⊗ [0, wx, wy, wz]  −  β * ∇f̂
 *   q   += qdot * dt,  then normalize
 *
 * where ∇f̂ is the normalized gradient of the objective function
 *   f(q) = q* ⊗ [0,0,0,1] ⊗ q  −  [ax, ay, az]
 * i.e. how far the quaternion-rotated gravity reference deviates from
 * the measured accelerometer direction.
 */
export class ImuTracker {
  constructor() {
    this._q    = new THREE.Quaternion();  // identity = flat/rest
    this._euler = new THREE.Euler();      // scratch: for degrees getter
  }

  reset() {
    this._q.identity();
  }

  /**
   * Bootstrap orientation from a single accelerometer sample.
   * Sets the initial tilt (pitch + roll) to match the measured gravity
   * direction so replay starts from the correct physical pose.
   * Yaw is not recoverable without a magnetometer and is assumed 0.
   *
   * @param {number[]} rawAccel raw int16 [ax, ay, az]
   */
  bootstrapFromAccel(rawAccel) {
    // Remap to child frame (same as update())
    const axc =  rawAccel[1] * ACCEL_SCALE;
    const ayc = -rawAccel[0] * ACCEL_SCALE;
    const azc =  rawAccel[2] * ACCEL_SCALE;
    const mag = Math.sqrt(axc*axc + ayc*ayc + azc*azc);

    if (mag < 0.3) { this._q.identity(); return; }

    // Find q such that q rotates the measured gravity direction to world-up [0,0,1].
    // Equivalently: q^{-1} * [0,0,1] = measured  →  this is what the Madgwick
    // objective function minimises at steady state.
    const measured = new THREE.Vector3(axc / mag, ayc / mag, azc / mag);
    this._q.setFromUnitVectors(measured, new THREE.Vector3(0, 0, 1));
  }

  /** @param {number[]} accel raw int16 [ax, ay, az]
   *  @param {number[]} gyro  raw int16 [gx, gy, gz] */
  update(accel, gyro) {
    // ── Remap IMU body → child (visual) frame ─────────────────────────────
    const wx =  gyro[1] * GYRO_SCALE;   // gy_imu  → child X (pitch)
    const wy = -gyro[0] * GYRO_SCALE;   // -gx_imu → child Y (roll)
    const wz =  gyro[2] * GYRO_SCALE;   // gz_imu  → child Z (yaw)

    const axc =  accel[1] * ACCEL_SCALE;  // ay_imu  → child X
    const ayc = -accel[0] * ACCEL_SCALE;  // -ax_imu → child Y
    const azc =  accel[2] * ACCEL_SCALE;  // az_imu  → child Z

    const { w: qw, x: qx, y: qy, z: qz } = this._q;

    // ── Gyro rate of change: qdot = 0.5 * q ⊗ [0, wx, wy, wz] ────────────
    let dqw = 0.5 * (-qx*wx - qy*wy - qz*wz);
    let dqx = 0.5 * ( qw*wx + qy*wz - qz*wy);
    let dqy = 0.5 * ( qw*wy - qx*wz + qz*wx);
    let dqz = 0.5 * ( qw*wz + qx*wy - qy*wx);

    // ── Madgwick gradient descent correction ──────────────────────────────
    const mag = Math.sqrt(axc*axc + ayc*ayc + azc*azc);
    if (mag > 0.3 && mag < 3.0) {   // skip during freefall or heavy vibration
      const ax = axc/mag, ay = ayc/mag, az = azc/mag;

      // Objective function: predicted gravity (z-up ref) vs measured accel.
      // Derived from rotating [0,0,1] from world→body via q, then subtracting
      // the normalised measured accel (Madgwick 2010, eq. 25).
      const f1 = 2*(qx*qz - qw*qy) - ax;
      const f2 = 2*(qw*qx + qy*qz) - ay;
      const f3 = 2*(0.5 - qx*qx - qy*qy) - az;

      // Gradient: J_g^T * f  (Madgwick 2010, eq. 26)
      let gw = -2*qy*f1 + 2*qx*f2;
      let gx =  2*qz*f1 + 2*qw*f2 - 4*qx*f3;
      let gy = -2*qw*f1 + 2*qz*f2 - 4*qy*f3;
      let gz =  2*qx*f1 + 2*qy*f2;

      // Normalise gradient then subtract β * ∇f̂ from qdot
      const gm = Math.sqrt(gw*gw + gx*gx + gy*gy + gz*gz);
      dqw -= BETA * gw / gm;
      dqx -= BETA * gx / gm;
      dqy -= BETA * gy / gm;
      dqz -= BETA * gz / gm;
    }

    // ── Integrate and normalise ────────────────────────────────────────────
    this._q.set(
      qx + dqx * DT,
      qy + dqy * DT,
      qz + dqz * DT,
      qw + dqw * DT,
    ).normalize();
  }

  /** Apply current orientation to a Three.js Object3D (the handGroup child). */
  applyTo(obj) {
    obj.quaternion.copy(this._q);
  }

  /** Euler angles in degrees relative to rest pose — for the debug panel. */
  get degrees() {
    this._euler.setFromQuaternion(this._q);
    const r2d = 180 / Math.PI;
    return [this._euler.x * r2d, this._euler.y * r2d, this._euler.z * r2d];
  }
}
