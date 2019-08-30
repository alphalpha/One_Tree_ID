#include <U8g2lib.h>
#include <RTClib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

RTC_DS1307 rtc;

////// CONFIG ////////////////////////
const int PIEZO_PIN = 11;
const int START_BUTTON_PIN = 8;
const int STOP_BUTTON_PIN = 9;

const int UPDATE_TIME = 4; // Time between display updates in seconds
const int START_TONE = 880; // In Hz
const int STOP_TONE = 659; // In Hz

const auto TITLE = "One Tree ID";
const auto TREE = "Black Pine";


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
  u8g2.begin();

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
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_t0_11_mf);
    u8g2.setCursor(0, 10);
    u8g2.print(TITLE);
    u8g2.setCursor(0, 27);
    u8g2.print(TREE);
    u8g2.setCursor(0, 44);
    u8g2.print("Linz, AT, " + started_at.timestamp(DateTime::TIMESTAMP_DATE));
    u8g2.setCursor(0, 61);
    if (!is_init_state) {
      if (show_time) {
        u8g2.print("Start: " + started_at.timestamp(DateTime::TIMESTAMP_TIME));
      } else {
        u8g2.print("Experiment No. " + String(counter));
      }
    }
  } while( u8g2.nextPage() );
  Serial.println("UPDATE DISPLAY");
}
