#include "wled.h"
#include "sensor_bus.h"

/*
 * ESP32 internal die temperature sensor provider.
 *
 * Reads the SoC's own built-in temperature sensor via the Arduino-ESP32
 * core's temperatureRead() (no external hardware, no I2C, no extra
 * library) and pushes it into the Sensor Hub (see
 * ../sensor-hub/usermod_sensor_hub.cpp and ../sensor-hub/sensor_bus.h) as
 * "<prefix>_temperature". This usermod never talks to MQTT, the JSON API
 * or the Info tab itself - the hub takes care of all of that once a
 * sensor is registered here.
 *
 * This reads the chip's own die temperature, which runs several degrees
 * above ambient once WiFi/CPU load heats up the SoC - it reflects module
 * temperature, not room temperature. ESP32-only (temperatureRead() is not
 * available on ESP8266); this usermod is a no-op there.
 */
class InternalTemperatureSensorUsermod : public Usermod {
  private:
    SensorHub* hub = nullptr;
    uint8_t tempHandle = SENSOR_HANDLE_INVALID;

    bool enabled = true;
    unsigned long lastRead = 0;

    // config
    uint16_t checkIntervalS = 30;            // how often to read the sensor
    String namePrefix = "internal";          // sensor name becomes "<prefix>_temperature"
    uint8_t precision = 1;                   // decimal places published

    static const char _name[];
    static const char _enabled[];
    static const char _checkInterval[];
    static const char _namePrefix[];
    static const char _precision[];

    void registerSensors() {
      if (!hub || tempHandle != SENSOR_HANDLE_INVALID) return; // already registered
      tempHandle = hub->registerSensor((namePrefix + "_temperature").c_str(), SensorType::Temperature, nullptr, nullptr, precision);
    }

  public:
    void setup() override {
#ifndef ARDUINO_ARCH_ESP32
      enabled = false; // internal die sensor is ESP32-only
#endif
    }

    void loop() override {
#ifdef ARDUINO_ARCH_ESP32
      if (!enabled) return;

      if (!hub) hub = getSensorHub(); // Sensor Hub usermod may finish init after us
      if (hub) registerSensors();

      unsigned long now = millis();
      if (now - lastRead < (unsigned long)checkIntervalS * 1000UL) return;
      lastRead = now;

      float t = temperatureRead();
      if (hub && tempHandle != SENSOR_HANDLE_INVALID) {
        hub->setSensorAvailable(tempHandle, true);
        hub->updateSensor(tempHandle, t);
      }
#endif
    }

    void addToConfig(JsonObject& root) override {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
      top[FPSTR(_checkInterval)] = checkIntervalS;
      top[FPSTR(_namePrefix)] = namePrefix;
      top[FPSTR(_precision)] = precision;
    }

    bool readFromConfig(JsonObject& root) override {
      JsonObject top = root[FPSTR(_name)];
      bool configComplete = !top.isNull();
      configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);
      configComplete &= getJsonValue(top[FPSTR(_checkInterval)], checkIntervalS);
      configComplete &= getJsonValue(top[FPSTR(_namePrefix)], namePrefix);
      configComplete &= getJsonValue(top[FPSTR(_precision)], precision);
      return configComplete;
    }

    void appendConfigData(Print& settingsScript) override {
      settingsScript.print(F("addInfo('InternalTemperatureSensor:checkInterval',1,'seconds between sensor reads');"));
      settingsScript.print(F("addInfo('InternalTemperatureSensor:namePrefix',1,'sensor name becomes &lt;prefix&gt;_temperature - must be unique across all sensor providers');"));
      settingsScript.print(F("addInfo('InternalTemperatureSensor:precision',1,'decimal places published');"));
    }
};

const char InternalTemperatureSensorUsermod::_name[]          PROGMEM = "InternalTemperatureSensor";
const char InternalTemperatureSensorUsermod::_enabled[]       PROGMEM = "enabled";
const char InternalTemperatureSensorUsermod::_checkInterval[] PROGMEM = "checkInterval";
const char InternalTemperatureSensorUsermod::_namePrefix[]    PROGMEM = "namePrefix";
const char InternalTemperatureSensorUsermod::_precision[]     PROGMEM = "precision";

static InternalTemperatureSensorUsermod internal_temperature_sensor;
REGISTER_USERMOD(internal_temperature_sensor);
