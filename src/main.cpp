#include <Arduino.h>

//=============Define Blynk====================
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPLimsqZyGu"
#define BLYNK_TEMPLATE_NAME "MiniGardenApartment"
#define BLYNK_AUTH_TOKEN "NiP5S5M_SBFiwWCEfI2MHorJ76YBeR-e"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/*===============INCLUDE LIB====================*/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "BitMap.h"
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <HTTPClient.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Hang_2.4G";
char pass[] = "0948315735";

/*====Define PIN=======*/
#define DHTPIN 5
#define DHTTYPE DHT11       // DHT 11
#define PUMP_RELAY_PIN 17   // chan kich bom nuoc
#define SOILMOISTURE_PIN 35 // chân độ ẩm đất
#define LDR_PIN 34          // chaan cam bien anh sang
#define TFT_W 128
#define TFT_H 128

/*=======BIEN TOAN CUC==========*/
float humi_value; // humidity
float temp_value;
int light_value;
int soil_value;
byte BT_STATE;
byte MANU_BT;
byte TIMER_BT;
long rtc_sec_server;
int s_hour_alr, s_minu_alr, s_sec_alr, r_hour_alr, r_minu_alr, r_sec_alr; // khai bao cac bien luu thoi gian
unsigned char weekDay, weekday_server;
bool flag_timer_en = 0; // co Timer on dung de dieu khieu MODE
bool flag_pump_status;
String GOOGLE_SCRIPT_ID = "AKfycbwF8kYY2g_p-Gy9Zmjowhgc0fFVg-BEqwwBn2KJJSACABtIjsytVB1K8VRWKP-jGJvD";
int count = 0 ;
/*========================*/

/*========TFT_PIN============
    GND
    VCC 3.3V
    SCL 18
    SDA 19
    RES 04
    DC  02
    CS  15
=============END_TFT============*/
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
TFT_eSPI tft = TFT_eSPI();
WidgetLED led_state(V8);
WidgetRTC rtc;

/*========== Ham con ======================*/
/*Doc gia tri lux cua cam bien anh sang*/
float getLightPercentage() // get do sang lux cua cam bien anh sang
{
  int ldrRawVal;
  float percentage;
  ldrRawVal = analogRead(LDR_PIN);
  float voltage = ldrRawVal / 4096. * 3.3;                 // tính điện áp ADC
  float resistance = 3000 * voltage / (1 - voltage / 3.3); // tính trở kháng ( theo tài liệu)
  float lux = pow(50 * 1e3 * pow(10, 0.61) / resistance, (1 / 0.61));
  // 0.7 là Độ dốc, 50 là trở LDR ở 10 lux, sử dụng cthuc nhà phát triển cấp
  return lux;
}
/*Doc gia tri cua cam bien do am dat*/
int getSoilMoisture(void)
{
  int Soil_Value = analogRead(SOILMOISTURE_PIN);
  int Soil_Final = map(Soil_Value, 0, 4095, 100, 0);
  return Soil_Final;
}

void sendSensor()
{
  humi_value = dht.readHumidity();
  temp_value = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  light_value = getLightPercentage();
  soil_value = getSoilMoisture();

  if (isnan(humi_value) || isnan(temp_value))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, humi_value);
  Blynk.virtualWrite(V0, temp_value);
  Blynk.virtualWrite(V3, light_value);
  Blynk.virtualWrite(V2, soil_value);
}
void PUMP_ON()
{
  led_state.on();
  digitalWrite(PUMP_RELAY_PIN, LOW);
  flag_pump_status = 1;
}
void PUMP_OFF()
{
  led_state.off();
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  flag_pump_status = 0;
}
int myWeekday()
{
  weekday_server = weekday();
  if (weekday_server == 1)
    weekday_server = 7;
  else
    weekday_server -= 1;

  return weekday_server;
}
void Mode()
{ // chọn chế độ làm việc là auto hoặc manual
  if (BT_STATE == 1)
  {
    if (soil_value <= 42 && temp_value <= 35 && light_value <= 400)
    { // thay đổi thông số mà bạn mong muốn hoạt động auto ở đây
      PUMP_ON();
    }
    else if (soil_value > 42)
    { // khi timer ko bat       //... cả chỗ này nữa
      PUMP_OFF();
    }
  }
  else if (BT_STATE == 0)
  {
    if (MANU_BT == 1)
    {
      PUMP_ON();
    }
    else
    {
      PUMP_OFF();
    }
  }
}
void Timer_Fn() // Function of Timer
{
  bool time_set_overflow;
  long start_timer_sec;
  long stop_timer_sec;

  time_set_overflow = 0;
  start_timer_sec = s_hour_alr * 3600 + s_minu_alr * 60 + s_sec_alr;
  stop_timer_sec = r_hour_alr * 3600 + r_minu_alr * 60 + r_sec_alr;

  if (stop_timer_sec < start_timer_sec)
    time_set_overflow = 1; // ktra tra gia tri sau >trc

  // if(hour() == s_hour_alr && (minute() >= s_minu_alr && minute() <= r_minu_alr ) && second()== s_sec_alr && (weekDay == 0x00 || (weekDay & (0x01 << (myWeekday()-1) ))) )  // test hen gio: pass
  // {

  //   Serial.println("BAT BOM ROI NE");

  // }else if(hour() == r_hour_alr && minute() == r_minu_alr && second()== r_sec_alr && (weekDay == 0x00 || (weekDay & (0x01 << (myWeekday()-1) ))))  // test hen gio: pass
  // {

  //   Serial.println("TAT BOM ROI !!");
  // }
  if ((((time_set_overflow == 0 && (rtc_sec_server >= start_timer_sec) && (rtc_sec_server < stop_timer_sec)) ||
        (time_set_overflow && ((rtc_sec_server >= start_timer_sec) || (rtc_sec_server < stop_timer_sec)))) &&
       (weekDay == 0x00 || (weekDay & (0x01 << (myWeekday() - 1))))))
  {
    PUMP_ON();
  }
  else
  {
    PUMP_OFF();
  }
}

void clockDisplay() // Test dong ho
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year() + " " + weekday();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  rtc_sec_server = (hour() * 60 * 60) + (minute() * 60) + second();
}

//==================SCREEN==============
void TFT_Screen_Welcome(void)
{
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_DARKGREEN,TFT_YELLOW);
  tft.setCursor(4, 20);
  tft.setTextSize(2);
  tft.print("Nhom10 IOT");
  tft.drawXBitmap(32,63,garden,64,64,TFT_DARKGREEN);
  delay(2000);
}
void TFT_Screen_Setup(void)
{
  tft.fillScreen(TFT_WHITE);
  tft.drawXBitmap(10, 5, temp1, 40, 40, TFT_RED);
  tft.drawXBitmap(80, 5, humi, 40, 40, TFT_BLUE);
  tft.drawXBitmap(15, 68, soil, 32, 32, TFT_BROWN);
  tft.drawXBitmap(80, 68, sun, 32, 32, TFT_ORANGE);
}

void TFT_Screen_Main(void)
{
  tft.fillRect(10, 48, 46, 15, TFT_WHITE); // nhiet do
  tft.fillRect(78, 48, 45, 15, TFT_WHITE); // do am ko khi
  tft.fillRect(10, 105, 42, 15, TFT_WHITE);
  tft.fillRect(80, 105, 47, 15, TFT_WHITE);
  // Nhiet do
  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);
  tft.drawFloat(temp_value, 1, 10, 48);
  tft.setCursor(58, 48);
  tft.setTextSize(1);
  tft.printf("%cC", 248);
  // Do am ko khi
  tft.setTextColor(TFT_BLUE);
  tft.setTextSize(2);
  tft.drawNumber(humi_value, 78, 48);
  tft.setCursor(115, 48);
  tft.print("%");
  // Do am dat
  tft.setTextColor(TFT_BROWN);
  tft.setTextSize(2);
  tft.drawNumber(soil_value, 12, 105);
  tft.setCursor(48, 105);
  tft.print("%");
  // Anh sang
  tft.setTextColor(TFT_ORANGE);
  tft.setTextSize(1);
  tft.drawNumber(light_value, 82, 105);
  tft.setCursor(90, 115);
  tft.setTextSize(1);
  tft.print("lux");
}
//========================
void Google_Sheet ()
{
  String lightPer_s(light_value);
  String tempPer_s(temp_value);
  String humiPer_s(humi_value);
  String soilPer_s(soil_value);
  String pumpStt_s(flag_pump_status);

  String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+ "stt=" + count + "&temp=" + tempPer_s + "&humi=" + humiPer_s + "&soil=" + soilPer_s + "&light=" + lightPer_s + "&pump=" + pumpStt_s;
  Serial.print("POST data to google sheet: "); Serial.println(urlFinal);
  HTTPClient http;
  http.begin(urlFinal.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Code Status: ");
  Serial.println(httpCode);
  // getting Response from GG Sheet
  String payload;
  if (httpCode > 0)
  {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  http.end();
  count++;
}
/*=============KET THUC HAM CON==============*/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(SOILMOISTURE_PIN, INPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  // Setup the TFT_LCD
  tft.init();
  tft.setRotation(1); // nam ngang
  TFT_Screen_Welcome();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  dht.begin();

  TFT_Screen_Setup();
  // Setup a function to be called every second
  timer.setInterval(3000L, sendSensor);
  timer.setInterval(3000L, TFT_Screen_Main);
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(60000L,Google_Sheet);
}

void loop()
{
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();

  if (flag_timer_en == 1)
  {
    Timer_Fn();
  }
  else
  {
    Mode();
  }
}
// ######################################################################
BLYNK_CONNECTED()
{
  Serial.print("BLYNK SERVER CONNECTED !!!");
  // Blynk.syncAll();
  rtc.begin();
  Blynk.syncVirtual(V5); // MODE Button
  Blynk.syncVirtual(V6); // COng tac
  Blynk.syncVirtual(V7); // Timer
  Blynk.syncVirtual(V9); // PUMP_BT
  Blynk.sendInternal("rtc", "sync");
}
// ######################################################################
/*=================*/
BLYNK_WRITE(V6)
{ // đọc gtri nút từ app blynk
  MANU_BT = param.asInt();
  Serial.println(MANU_BT);
}
BLYNK_WRITE(V5)
{ // auto hoac manual
  BT_STATE = param.asInt();
  Serial.println(BT_STATE);
}
//==================
BLYNK_WRITE(V7)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    Serial.println(String("Start: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    s_hour_alr = t.getStartHour();
    s_minu_alr = t.getStartMinute();
    s_sec_alr = t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
    Serial.println("Start at sunrise");
  }
  else if (t.isStartSunset())
  {
    Serial.println("Start at sunset");
  }
  else
  {
    // Do nothing
  }

  // Process stop time

  if (t.hasStopTime())
  {
    Serial.println(String("Stop: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
    r_hour_alr = t.getStopHour();
    r_minu_alr = t.getStopMinute();
    r_sec_alr = t.getStopSecond();
  }
  else if (t.isStopSunrise())
  {
    Serial.println("Stop at sunrise");
  }
  else if (t.isStopSunset())
  {
    Serial.println("Stop at sunset");
  }
  else
  {
    // Do nothing: no stop time was set
  }

  // Process timezone
  // Timezone is already added to start/stop time

  Serial.println(String("Time zone: ") + t.getTZ());

  // Get timezone offset (in seconds)
  Serial.println(String("Time zone offset: ") + t.getTZ_Offset());

  for (int i = 1; i <= 7; i++)
  {
    if (t.isWeekdaySelected(i)) // will be "TRUE" if nothing selected as well
    {
      weekDay |= (0x01 << (i - 1));
    }
    else
      weekDay &= (~(0x01 << (i - 1)));
  }

  Serial.print("Time1 Selected Days: ");
  Serial.println(weekDay, HEX);

  Serial.println();
}
BLYNK_WRITE(V9)
{
  TIMER_BT = param.asInt();
  Serial.println(TIMER_BT);
  if (TIMER_BT == 0)
  {
    Serial.println("Che Do Hen Gio da TAT");
    flag_timer_en = 0;
  }
  else
  {
    Serial.println("Che Do Hen Gio da BAT");
    flag_timer_en = 1;
  }
}