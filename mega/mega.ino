#include <Thread.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv tft;

#include <Fonts/FreeMono9pt7b.h>

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

const int btn_vent = 22;
const int btn_hum = 24;
const int btn_light = 26;

const int pin_vent = 23;
const int pin_light = 27;

bool vent_state = false;
bool hum_state = false;
bool light_state = false;

float temp = 0;
float hum = 0;
float wet = 0;

Thread threadDisplay = Thread();
Thread threadState = Thread();
Thread threadShareState = Thread();

void setup(void)
{
  Serial1.begin(9600);
  Serial.begin(9600);
  initDisplay();

  pinMode(btn_vent, INPUT);
  pinMode(btn_hum, INPUT);
  pinMode(btn_light, INPUT);

  threadDisplay.setInterval(seconds(5));
  threadDisplay.onRun(showInfo);

  threadState.setInterval(100);
  threadState.onRun(checkState);

  threadShareState.setInterval(seconds(1));
  threadShareState.onRun(shareState);
}

void initDisplay()
{
  uint16_t ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.setTextSize(5);
  analogWrite(11, 150);
  tft.setTextColor(TFT_DARKGREY); //  tft.setTextColor(TFT_DARKGREEN);

  showmsgXY(5, 50, 2, &FreeMono9pt7b, "Temp:");
  showmsgXY(5, 100, 2, &FreeMono9pt7b, "Hum:");
  showmsgXY(5, 150, 2, &FreeMono9pt7b, "Ventilation:");
  showmsgXY(5, 200, 2, &FreeMono9pt7b, "Humadiation:");
  showmsgXY(5, 250, 2, &FreeMono9pt7b, "Light:");
  showInfo();
}

void loop(void)
{
  if (threadDisplay.shouldRun()) { threadDisplay.run(); }
  if (threadState.shouldRun()) { threadState.run(); }
  if (threadShareState.shouldRun()) { threadShareState.run(); }
  while (Serial.available()) { updateStatus(Serial.readString()); }
}

void checkState()
{
  bool changed = false;

  if (digitalRead(btn_vent)) { vent_state = !vent_state; changed = true; };
  if (digitalRead(btn_hum)) { hum_state = !hum_state; changed = true; };
  if (digitalRead(btn_light)) { light_state = !light_state; changed = true; };

  if (changed) {
    digitalWrite(pin_vent, vent_state);
    digitalWrite(pin_light, light_state);
    threadDisplay.run();
  }
}

void shareState()
{
  Serial.println("vent_" + String(vent_state));
  Serial.println("hum__" + String(hum_state));
}

void updateStatus(String cmd) {
  cmd.trim();
  String name = cmd.substring(0, 6);
  String value = cmd.substring(7);

  if (name == "s_temp_") { temp = value.toFloat(); }
  if (name == "s_hum__") { hum = value.toFloat();; }
  if (name == "s_wet__") { wet = value.toFloat();; }

  threadDisplay.run();
}

void showInfo()
{
  tft.fillRect(270, 5, 470, 200, BLACK);

  showmsgXY(280, 50, 2, &FreeMono9pt7b, String(temp));
  showmsgXY(280, 100, 2, &FreeMono9pt7b, String(hum));
  showmsgXY(280, 150, 2, &FreeMono9pt7b, stateStr(vent_state));
  showmsgXY(280, 200, 2, &FreeMono9pt7b, stateStr(hum_state));
  showmsgXY(280, 250, 2, &FreeMono9pt7b, stateStr(light_state));
}

// =============================HELPERS===================================

void showmsgXY(int x, int y, int sz, const GFXfont *f, const String msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);

  tft.setTextSize(sz);
  tft.print(msg);
}

String stateStr(bool state) {
  return(state ? "ON" : "OFF");
}

bool strState(String value) {
  return((value == "1") ? true : false);
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
