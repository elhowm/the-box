#include <DHT.h>
#include <Thread.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define WETPIN A0

DHT dht(DHTPIN, DHTTYPE);

float temp = 0;
float hum = 0;
float wet = 0;

const int pin_hum = 5;
const int pin_pump = 6;
const int pin_vent = 7;
const int pin_error = 9;

bool vent_state = false;
bool hum_state = false;

String command = "";

Thread threadSensors = Thread();

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(pin_hum, OUTPUT);
  pinMode(pin_pump, OUTPUT);
  pinMode(pin_vent, OUTPUT);
  pinMode(pin_error, OUTPUT);

  threadSensors.setInterval(seconds(10));
  threadSensors.onRun(loadSensors);
}

void loop() {
  if (threadSensors.shouldRun()) { threadSensors.run(); }

  while (Serial.available()) {
    command = Serial.readStringUntil('\n');
  };

  if (command != "") { updateState(command); };

  digitalWrite(pin_vent, vent_state);
}

void shareState()
{
  Serial.println("s_temp_" + String(hum));
  Serial.println("s_hum__" + String(temp));
  Serial.println("s_wet__" + String(wet));
}

void loadSensors() {
  hum = dht.readHumidity();
  temp = dht.readTemperature(false);
  wet = convertToPercent(analogRead(WETPIN));
  shareState();
}

int convertToPercent(int value) {
  int percentValue = 0;
  percentValue = map(value, 1023, 465, 0, 100);
  return percentValue;
}

void updateState(String cmd) {
  cmd.trim();
  String name = cmd.substring(0, 5);
  String value = cmd.substring(5);
  
  if (name == "vent_") { vent_state = strToBool(value); }
  if (name == "hum__") { hum_state = strToBool(value); }

  Serial.println(vent_state ? "true" : "false");
  delay(1); // for stable 
}

// =============================HELPERS===================================

bool strToBool(String value) {
  return(value.equals("1"));
}

long minutes(int count)
{
  return count * 60 * 1000L;
}

long seconds(int count)
{
  return count * 1000L;
}

unsigned long hours(int count)
{
  return count * minutes(60);
}
