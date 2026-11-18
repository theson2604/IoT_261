# YoloUNO IoT exercises

Open this folder in PlatformIO. The three environments use the same YoloUNO board configuration.

| Environment | Exercise |
| --- | --- |
| `blink_dht2` | Blink the LEDs and read DHT20 temperature and humidity over I2C |
| `binary_semaphore` | Signal the LED task from the BOOT button using a FreeRTOS binary semaphore |
| `webserver` | Serve desktop and mobile dashboards with DHT20 readings and synchronized LED controls |

`webserver` is the default. To build or upload an exercise, select its environment in PlatformIO or use:

```text
pio run -e blink_dht2
pio run -e blink_dht2 -t upload
```

If automatic port detection fails, add `--upload-port COM4` to the upload command with your board's COM port. Serial Monitor runs at 115200 baud.

After uploading `webserver`, join the board's Wi-Fi network and open the address printed in Serial Monitor. To forward USB readings from `blink_dht2` to Core IoT, see [the serial bridge instructions](SERIAL_TO_COREIOT.md).
