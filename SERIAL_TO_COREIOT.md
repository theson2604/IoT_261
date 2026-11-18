# Send USB sensor readings to Core IoT

Data path: DHT20 -> Yolo UNO -> USB Serial -> Python on the computer -> Core IoT.
The computer needs internet access. The board needs no Wi-Fi configuration.

1. Upload the `blink_dht2` environment.
2. Close PlatformIO Serial Monitor and stop the previous Python sample. Only one
   program can open the board's COM port, and the sample sends simulated values
   to the same device and uses the same MQTT client ID.
3. Open a terminal in this project folder and run:

```powershell
py -m pip install -r requirements.txt
$env:COREIOT_ACCESS_TOKEN = Read-Host 'Core IoT device password' -MaskInput
py serial_to_coreiot.py --port COM4
```

Replace COM4 if the board has a different port. List ports with
`py -m serial.tools.list_ports`.

The board sends real JSON readings every second, for example:
`{"temperature":30.25,"humidity":65.40}`.
The bridge publishes a real reading every five seconds to
`v1/devices/me/telemetry` on `app.coreiot.io:1883`, using username
`test_device_2` and the credential from `COREIOT_ACCESS_TOKEN`. Publishing uses QoS 1.
It ignores board startup/counter/LED text and prints `Published: ...` after
broker acknowledgement. View temperature and humidity under the device's latest
telemetry in Core IoT. No simulated light value or RPC handling is added.

The bridge retries MQTT and serial connections. Keep it running to send data;
press Ctrl+C before uploading new firmware or reopening Serial Monitor.
It forwards live readings; it does not archive all readings during an outage.
Use `--interval 1` to publish every second instead.

Python dependencies: paho-mqtt 2.x (version 2 callbacks) and pyserial 3.x.
References: https://eclipse.dev/paho/files/paho.mqtt.python/html/client.html
and https://pyserial.readthedocs.io/en/latest/pyserial_api.html.
