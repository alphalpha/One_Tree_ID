#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RTC_DS1307 rtc;

////// CONFIG ////////////////////////
const int PIEZO_PIN = 11;
const int START_BUTTON_PIN = 8;
const int STOP_BUTTON_PIN = 9;

const int UPDATE_TIME = 4; // Time between display updates in seconds
const int START_TONE = 880; // In Hz
const int STOP_TONE = 659; // In Hz

const auto TITLE = "One Tree ID";
const auto TREE = "Pinus Nigra";


//////////////////////////////////////
int counter = 0;
DateTime updated_at;
DateTime started_at;
auto is_init_state = true;
bool show_time = false;
bool started = false;
bool stopped = false;
bool start_button_pressed = false;
bool stop_button_pressed = false;

void setup() {
  Serial.begin(9600);

  // Set Up Clock
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while(1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  started_at = rtc.now();
  updated_at = started_at;

  // Set Up Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  delay(2000);
  //  display.setFont(&Roboto_Mono_Light_8);

  pinMode(PIEZO_PIN, OUTPUT);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  const auto current_time = rtc.now();
  const auto start_button_pressed = !digitalRead(START_BUTTON_PIN);
  const auto stop_button_pressed = !digitalRead(STOP_BUTTON_PIN);
  
  if (start_button_pressed && !started) {
    Serial.println("START BUTTON PRESSED");
    started_at = current_time;
    updated_at = started_at;
    ++counter;
    started = true;
    show_time = false;
    is_init_state = false;
    
    update_display();
    
    tone(PIEZO_PIN, START_TONE, 200);
    Serial.println();
  } else if (!start_button_pressed) {
    started = false;
  }

  if (stop_button_pressed && !stopped) {
    Serial.println("STOP BUTTON PRESSED");
    stopped = true;

    tone(PIEZO_PIN, STOP_TONE, 200);
    Serial.println();
  } else if (!stop_button_pressed) {
    stopped = false;
  }

  if ((updated_at + TimeSpan(UPDATE_TIME)) < current_time) {
    show_time = !show_time;
    updated_at = current_time;
    
    update_display();
    Serial.println();
  }
  delay(100);
}

void update_display() {
  display.clearDisplay();
//  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print(TITLE);
  
  display.setCursor(0, 25);
  display.print(TREE);
  
  display.setCursor(0, 40);
  display.print("Linz, AT, " + started_at.timestamp(DateTime::TIMESTAMP_DATE));
  
  display.setCursor(0, 55);
  if (!is_init_state) {
    if (show_time) {
      display.print("Start: " + started_at.timestamp(DateTime::TIMESTAMP_TIME));
    } else {
      display.print("Experiment No. " + String(counter));
    }
  }  
  display.display();
  Serial.println("UPDATE DISPLAY");
}
