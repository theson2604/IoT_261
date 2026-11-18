"""Forward Yolo UNO USB serial temperature/humidity readings to Core IoT."""

import argparse
import json
import math
import os
import threading
import time

import paho.mqtt.client as mqttclient
import serial

BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
ACCESS_USERNAME = "test_device_2"
TELEMETRY_TOPIC = "v1/devices/me/telemetry"


def parse_telemetry(line):
    """Accept sensor JSON only; ignore startup messages and LED/counter logs."""
    try:
        data = json.loads(line)
    except (ValueError, UnicodeDecodeError):
        return None
    if not isinstance(data, dict):
        return None
    values = {}
    for key in ("temperature", "humidity"):
        value = data.get(key)
        if type(value) not in (int, float):
            return None
        try:
            if not math.isfinite(value):
                return None
        except OverflowError:
            return None
        values[key] = value
    return values


def connected(client, ready, flags, reason_code, properties):
    if reason_code == 0:
        ready.set()
        print("Connected to Core IoT as test_device_2!", flush=True)
    else:
        ready.clear()
        print(f"MQTT connection rejected: {reason_code}", flush=True)


def disconnected(client, ready, flags, reason_code, properties):
    ready.clear()
    if reason_code != 0:
        print(f"MQTT disconnected: {reason_code}; retrying...", flush=True)


def connection_failed(client, ready):
    ready.clear()
    print("Cannot reach Core IoT; check the computer's internet. Retrying...", flush=True)


def forward_serial(client, ready, port_name, baud, interval):
    last_publish = float("-inf")
    while True:
        try:
            with serial.Serial(port_name, baud, timeout=0.5) as board:
                print(f"Reading {port_name} at {baud} baud. Press Ctrl+C to stop.", flush=True)
                pending = b""
                while True:
                    # Preserve partial lines across USB reads and read timeouts.
                    pending += board.read(min(board.in_waiting or 1, 4096))
                    while b"\n" in pending:
                        raw_line, pending = pending.split(b"\n", 1)
                        line = raw_line.decode("utf-8", errors="replace").strip()
                        if not line:
                            continue
                        reading = parse_telemetry(line)
                        if reading is None:
                            print(f"Board: {line}", flush=True)
                            continue
                        print(f"Sensor: {reading}", flush=True)
                        if not ready.is_set() or time.monotonic() - last_publish < interval:
                            continue
                        payload = json.dumps(reading, allow_nan=False)
                        last_publish = time.monotonic()
                        info = client.publish(TELEMETRY_TOPIC, payload, qos=1)
                        if info.rc != mqttclient.MQTT_ERR_SUCCESS:
                            print(f"Publish failed: {mqttclient.error_string(info.rc)}", flush=True)
                            continue
                        try:
                            info.wait_for_publish(timeout=5)
                            if info.is_published():
                                print(f"Published: {payload}", flush=True)
                            else:
                                print("Waiting for broker acknowledgement; MQTT will retry.", flush=True)
                        except (RuntimeError, ValueError) as exc:
                            print(f"Publish interrupted: {exc}", flush=True)
                    if len(pending) > 4096:
                        pending = b""  # Discard malformed output without a newline.
        except (serial.SerialException, OSError) as exc:
            print(f"Serial error: {exc}", flush=True)
            print("Close Serial Monitor, check the COM port/cable. Retrying in 2 s...", flush=True)
            time.sleep(2)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="COM4", help="Board serial port (default: COM4)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--interval", type=float, default=5, help="Publish interval in seconds")
    args = parser.parse_args()
    if not math.isfinite(args.interval) or args.interval <= 0 or args.baud <= 0:
        parser.error("--interval and --baud must be positive")

    ready = threading.Event()
    client = mqttclient.Client(
        callback_api_version=mqttclient.CallbackAPIVersion.VERSION2,
        client_id="TEST_DEVICE_2",
        userdata=ready,
    )
    token = os.environ.get("COREIOT_ACCESS_TOKEN")
    if not token:
        parser.error("Set the COREIOT_ACCESS_TOKEN environment variable.")
    client.username_pw_set(ACCESS_USERNAME, token)
    client.on_connect = connected
    client.on_disconnect = disconnected
    client.on_connect_fail = connection_failed
    client.reconnect_delay_set(min_delay=1, max_delay=30)
    client.max_inflight_messages_set(1)
    client.max_queued_messages_set(1)
    client.connect_async(BROKER_ADDRESS, PORT, keepalive=60)
    client.loop_start()
    try:
        forward_serial(client, ready, args.port, args.baud, args.interval)
    except KeyboardInterrupt:
        print("\nStopping serial bridge.", flush=True)
    finally:
        client.disconnect()
        client.loop_stop()


if __name__ == "__main__":
    main()
