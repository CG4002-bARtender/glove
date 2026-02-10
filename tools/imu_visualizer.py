#!/usr/bin/env python3
"""
IMU 3D Visualizer - Subscribes to MQTT, applies sensor fusion, renders 3D box.

Usage:
    cd tools && uv run streamlit run imu_visualizer.py
"""

import json
import math
import ssl
import time
import threading
from collections import deque

import numpy as np
import paho.mqtt.client as mqtt
import plotly.graph_objects as go
import streamlit as st
from scipy.spatial.transform import Rotation as R

# MQTT config (mirrors config.h)
BROKER = "k12141b9.ala.eu-central-1.emqxsl.com"
PORT = 8883
USERNAME = "test"
PASSWORD = "test"
TOPIC = "glove/imu"

# MPU6050 default scale factors
ACCEL_SCALE = 16384.0  # LSB/g for +/-2g
GYRO_SCALE = 131.0     # LSB/(deg/s) for +/-250deg/s

ALPHA = 0.98
CALIBRATION_SECONDS = 3.0

# Box vertices (centered at origin, size 4x1x2)
BOX_VERTICES = np.array([
    [-2, -0.5, -1], [ 2, -0.5, -1], [ 2,  0.5, -1], [-2,  0.5, -1],  # back face
    [-2, -0.5,  1], [ 2, -0.5,  1], [ 2,  0.5,  1], [-2,  0.5,  1],  # front face
], dtype=float)

# Triangular faces for Mesh3d (each quad = 2 triangles)
BOX_I = [0, 0, 4, 4, 0, 0, 1, 1, 0, 0, 3, 3]
BOX_J = [1, 2, 5, 6, 1, 4, 2, 5, 3, 4, 2, 6]
BOX_K = [2, 3, 6, 7, 4, 5, 5, 6, 4, 7, 6, 7]


class ImuFusion:
    """Complementary filter for roll/pitch/yaw from 6-axis IMU."""

    def __init__(self):
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        self.last_t = None
        self.gyro_bias = np.zeros(3)

    def calibrate(self, samples):
        if not samples:
            return

        accel = np.array([s["a"] for s in samples], dtype=float)
        gyro = np.array([s["g"] for s in samples], dtype=float)

        self.gyro_bias = gyro.mean(axis=0)

        avg_a = accel.mean(axis=0) / ACCEL_SCALE
        self.roll = math.degrees(math.atan2(avg_a[1], avg_a[2]))
        self.pitch = math.degrees(math.atan2(-avg_a[0], math.sqrt(avg_a[1]**2 + avg_a[2]**2)))
        self.yaw = 0.0
        self.last_t = samples[-1]["t"]

    def update(self, sample):
        t = sample["t"]
        if self.last_t is None:
            self.last_t = t
            return
        dt = (t - self.last_t) / 1000.0
        self.last_t = t

        if dt <= 0 or dt > 0.5:
            return

        gyro_dps = (np.array(sample["g"], dtype=float) - self.gyro_bias) / GYRO_SCALE
        accel_g = np.array(sample["a"], dtype=float) / ACCEL_SCALE

        accel_roll = math.degrees(math.atan2(accel_g[1], accel_g[2]))
        accel_pitch = math.degrees(math.atan2(-accel_g[0], math.sqrt(accel_g[1]**2 + accel_g[2]**2)))

        self.roll = ALPHA * (self.roll + gyro_dps[0] * dt) + (1 - ALPHA) * accel_roll
        self.pitch = ALPHA * (self.pitch + gyro_dps[1] * dt) + (1 - ALPHA) * accel_pitch
        self.yaw += gyro_dps[2] * dt

    def rotation(self):
        return R.from_euler("xyz", [self.roll, self.pitch, self.yaw], degrees=True)


class MqttImuSubscriber:
    """Thread-safe MQTT subscriber that queues incoming IMU samples."""

    def __init__(self, broker, port, username, password, topic):
        self.topic = topic
        self.queue = deque(maxlen=500)
        self.samples = deque(maxlen=5000)
        self.lock = threading.Lock()
        self.connected = False

        self.client = mqtt.Client(
            callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
            client_id="imu_visualizer",
            protocol=mqtt.MQTTv5,
        )
        self.client.username_pw_set(username, password)
        self.client.tls_set(tls_version=ssl.PROTOCOL_TLS_CLIENT)
        self.client.tls_insecure_set(True)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message

        self.client.connect(broker, port, keepalive=60)
        self.client.loop_start()

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        client.subscribe(self.topic)
        self.connected = True

    def _on_message(self, client, userdata, msg):
        try:
            data = json.loads(msg.payload)
            with self.lock:
                self.queue.append(data)
                self.samples.append(data)
        except json.JSONDecodeError:
            pass

    def drain_queue(self):
        with self.lock:
            items = list(self.queue)
            self.queue.clear()
            return items

    def has_samples(self):
        with self.lock:
            return len(self.samples) > 0

    def get_all_samples(self):
        with self.lock:
            return list(self.samples)

    def clear_samples(self):
        with self.lock:
            self.samples.clear()
            self.queue.clear()

    def stop(self):
        self.client.loop_stop()
        self.client.disconnect()


def make_box_figure(rot):
    """Create a Plotly 3D figure with a rotated box."""
    verts = rot.apply(BOX_VERTICES)

    fig = go.Figure(data=[
        go.Mesh3d(
            x=verts[:, 0], y=verts[:, 1], z=verts[:, 2],
            i=BOX_I, j=BOX_J, k=BOX_K,
            color="cyan", opacity=0.8, flatshading=True,
        ),
        # Direction arrow (rotated X axis)
        go.Scatter3d(
            x=[0, rot.apply([3, 0, 0])[0]],
            y=[0, rot.apply([3, 0, 0])[1]],
            z=[0, rot.apply([3, 0, 0])[2]],
            mode="lines", line=dict(width=8, color="red"),
            hoverinfo="none",
        ),
    ])

    fig.update_layout(
        scene=dict(
            xaxis=dict(range=[-4, 4], showspikes=False),
            yaxis=dict(range=[-4, 4], showspikes=False),
            zaxis=dict(range=[-4, 4], showspikes=False),
            aspectmode="cube",
            camera=dict(
                up=dict(x=0, y=0, z=1),
                eye=dict(x=1.5, y=1.5, z=1.0),
            ),
        ),
        margin=dict(l=0, r=0, b=0, t=0),
        height=600,
    )
    return fig


# -- Streamlit app --

st.set_page_config(page_title="IMU Visualizer", layout="wide")
st.markdown("## IMU 3D Visualizer")

# Persistent state across Streamlit reruns
if "sub" not in st.session_state:
    st.session_state.sub = None
    st.session_state.fusion = None
    st.session_state.phase = "connecting"

sub = st.session_state.sub
fusion = st.session_state.fusion
phase = st.session_state.phase

# Phase: connecting
if phase == "connecting":
    st.info("Connecting to MQTT broker...")
    sub = MqttImuSubscriber(BROKER, PORT, USERNAME, PASSWORD, TOPIC)
    st.session_state.sub = sub

    timeout = 10.0
    start = time.time()
    while not sub.connected and (time.time() - start) < timeout:
        time.sleep(0.1)

    if sub.connected:
        st.session_state.phase = "calibrating"
        st.rerun()
    else:
        st.error("Could not connect to MQTT broker.")
        st.stop()

# Phase: calibrating
if phase == "calibrating":
    st.warning(f"Hold the glove STILL for {CALIBRATION_SECONDS} seconds...")

    while not sub.has_samples():
        time.sleep(0.1)

    sub.clear_samples()
    time.sleep(CALIBRATION_SECONDS)

    fusion = ImuFusion()
    fusion.calibrate(sub.get_all_samples())
    sub.clear_samples()
    st.session_state.fusion = fusion
    st.session_state.phase = "streaming"
    st.rerun()

# Phase: streaming
if phase == "streaming":
    col1, col2 = st.columns([3, 1])
    with col2:
        st.markdown("### Orientation")
        angle_display = st.empty()
        rate_display = st.empty()
        st.markdown("---")
        if st.button("Recalibrate"):
            st.session_state.phase = "calibrating"
            st.rerun()

    with col1:
        chart = st.empty()

    frame_count = 0
    start_time = time.time()

    while True:
        new_samples = sub.drain_queue()
        for sample in new_samples:
            fusion.update(sample)

        if new_samples:
            rot = fusion.rotation()
            frame_count += 1
            chart.plotly_chart(make_box_figure(rot), width="stretch",
                              config={"displayModeBar": False},
                              key=f"imu_{frame_count}")

            angle_display.markdown(
                f"**Roll:** {fusion.roll:+.1f} deg  \n"
                f"**Pitch:** {fusion.pitch:+.1f} deg  \n"
                f"**Yaw:** {fusion.yaw:+.1f} deg"
            )

            elapsed = time.time() - start_time
            if elapsed > 0:
                rate_display.markdown(f"**Update rate:** {frame_count / elapsed:.1f} Hz")

        time.sleep(0.04)  # ~25 FPS cap
