#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define ONE_WIRE_BUS 2
#define PROM_URI "http://pushgw:9091/metrics/job/kegtemp/instance/kegtemp1"
#define WIFI_SSID "Brewery"
#define WIFI_PASS "P@ssw0rd"

#if DEBUG
  #define D_SerialBegin(...) Serial.begin(__VA_ARGS__);
  #define D_print(...)       Serial.print(__VA_ARGS__)
  #define D_println(...)     Serial.println(__VA_ARGS__)
#else
  #define D_SerialBegin(...)
  #define D_print(...)
  #define D_println(...)
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void sleep(unsigned int seconds) {
  D_println("Going to sleep 😴");
  ESP.deepSleep(seconds * 1e6);
}

void sendPrometheus(float temperature) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, PROM_URI);
  http.addHeader("User-Agent", "KegTemp");
  http.addHeader("Connection", "close");
  http.addHeader("Content-Type", "text/plain");

  String msg;
  msg += "# TYPE temperature gauge\n";
  msg += "# HELP temperature The approximate value of temperature.\n";
  msg += "temperature ";
  msg += String(temperature, 2);
  msg += "\n";

  int httpCode = http.POST(msg);
  D_print("Prometheus push GW response code: ");
  D_println(httpCode);

  http.end();
  client.stop();
}

void setup() {
  D_SerialBegin(115200);
  sensors.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  D_print("Connecting");
  int max_retries = 10; // In seconds
  while (WiFi.status() != WL_CONNECTED) {
    if (max_retries-- < 0) {
      D_println("Could'nt connect to WiFi.");
      sleep(30);
    }
    delay(1000);
    D_print(".");
  }
  D_println();
  D_print("Connected, IP address: ");
  D_println(WiFi.localIP());

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC != DEVICE_DISCONNECTED_C) {
    sendPrometheus(tempC);
    D_print("Temperature: ");
    D_print(tempC);
    D_println("°C");
  } else {
    D_println("Couldn't read temperature!");
  }

  sleep(300);
}

void loop() {
}
