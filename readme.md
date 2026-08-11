# Internal Temperature Sensor Provider

A [Sensor Hub](../sensor-hub/readme.md) provider usermod for the ESP32's
own built-in die temperature sensor - registers `internal_temperature`
with the hub by default, which then handles MQTT, Home Assistant
discovery, the JSON API and the Info tab.

No external hardware, no I2C, no library dependency - just the
Arduino-ESP32 core's `temperatureRead()`. ESP32-only; a no-op on ESP8266
builds.

Note this is the chip's own die temperature, which runs several degrees
above ambient once WiFi/CPU load heats up the SoC - it reflects module
temperature, not room temperature.

## Usage

Add `sensor-hub-internal-temperature-provider` to `custom_usermods` next to
the [Sensor Hub](../sensor-hub/readme.md) itself. No wiring, no settings
beyond the usual enable/interval/name/precision.

## Usermod Settings

| Setting | Default | Description |
|---|---|---|
| Enabled | on | Master on/off switch |
| Check interval | 30s | How often the sensor is read |
| Name prefix | `internal` | Sensor name becomes `<prefix>_temperature` - must be unique across every provider registered with the hub |
| Precision | 1 | Decimal places published |
