#include <DHT.h>
#include <Thread.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define WETPIN A0

DHT dht(DHTPIN, DHTTYPE);

const int PIN_HUM = 5;
const int PIN_PUMP = 6;
const int PIN_VENT = 7;
const int PIN_PWR_WET = 8;
const int PIN_PWR_DHT = 9;
const int PIN_ERROR = 10;

float temp = 0;
float hum = 0;
float wet = 0;

bool vent_state = false;
bool hum_state = false;
bool pump_state = false;

Thread threadSensors = Thread();

String command = "";

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(PIN_HUM, OUTPUT);
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_VENT, OUTPUT);
  pinMode(PIN_PWR_WET, OUTPUT);
  pinMode(PIN_PWR_DHT, OUTPUT);
  pinMode(PIN_ERROR, OUTPUT);

  threadSensors.setInterval(seconds(10));
  threadSensors.onRun(loadSensors);
}

void loop() {
  if (threadSensors.shouldRun()) { threadSensors.run(); }

  listenCommands();
}

void loadSensors() {
  digitalWrite(PIN_PWR_WET, HIGH);
  digitalWrite(PIN_PWR_DHT, HIGH);
  delay(1000);

  hum = dht.readHumidity();
  temp = dht.readTemperature();
  wet = convertWetToPercent(analogRead(WETPIN));
  digitalWrite(PIN_PWR_WET, LOW);
  digitalWrite(PIN_PWR_DHT, LOW);

  shareState();
}

void shareState()
{
  Serial.println("s_temp_" + String(temp));
  Serial.println("s_hum__" + String(hum));
  Serial.println("s_wet__" + String(wet));
}

void listenCommands() {
  command = "";

  while (Serial.available()) {
    command = Serial.readStringUntil('\n');
  };
  if (command != "") { updateState(command); };
}

void updateState(String cmd) {
  cmd.trim();
  String name = cmd.substring(0, 5);
  String value = cmd.substring(5);

  if (name == "vent_") { vent_state = strToBool(value); }
  if (name == "hum__") { hum_state = strToBool(value); }
  if (name == "pump_") { pump_state = strToBool(value); }

  digitalWrite(PIN_VENT, vent_state);
  digitalWrite(PIN_HUM, hum_state);
  digitalWrite(PIN_PUMP, pump_state);
  delay(1); // for stable
}

// =============================HELPERS===================================

int convertWetToPercent(int value) {
  int percentValue = 0;
  percentValue = map(value, 580, 230, 0, 100);
  return percentValue;
}

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
