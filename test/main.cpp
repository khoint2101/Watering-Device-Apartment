#include <Arduino.h>

//=============Define Blynk====================
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPLimsqZyGu"
#define BLYNK_TEMPLATE_NAME         "MiniGardenApartment"
#define BLYNK_AUTH_TOKEN            "NiP5S5M_SBFiwWCEfI2MHorJ76YBeR-e"

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

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

/*====Define PIN=======*/
#define DHTPIN 5
#define DHTTYPE DHT11     // DHT 11
#define PUMP_RELAY_PIN 17  // chan kich bom nuoc
#define SOILMOISTURE_PIN 35     //chân độ ẩm đất
#define LDR_PIN 34   // chaan cam bien anh sang

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

/*========== Ham con ======================*/
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V0, t);
}
        /*Doc gia tri lux cua cam bien anh sang*/
float getLightPercentage(void)   // get do sang lux cua cam bien anh sang
{
  int ldrRawVal;
  float percentage;
  ldrRawVal = analogRead(LDR_PIN);
  float voltage = ldrRawVal / 4096. * 3.3;    // tính điện áp ADC
  float resistance = 3000 * voltage / (1 - voltage / 3.3);  // tính trở kháng ( theo tài liệu)
  float lux = pow(50 * 1e3 * pow(10, 0.61) / resistance, (1 / 0.61)); 
  // 0.7 là Độ dốc, 50 là trở LDR ở 10 lux, sử dụng cthuc nhà phát triển cấp
  return lux;
}
        /*Doc gia tri cua cam bien do am dat*/
uint32_t getSoilMoisture(void)
{
  uint16_t Soil_Value = analogRead(SOILMOISTURE_PIN);
  uint16_t Soil_Final = map(Soil_Value,0,4095,0,100);
  return Soil_Final;
}
/*=============KET THUC HAM CON==============*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  dht.begin();

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}

