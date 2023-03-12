//Tek4.vn
//Thư viện yêu cầu phải có 
#include "WiFi.h"
#include <HTTPClient.h>
#define LDR_PIN       34          //chân LDR phân áp với trở nối với ESP

// Thông tin WIFI MODE STA
const char* ssid = "Tầng 4";         // Đổi SSID
const char* password = "Tanghai@";    // Đổi mật khẩu
// Thông tin ID của GG Script
String GOOGLE_SCRIPT_ID = "AKfycbyA2_GCpR0PqcAkzMy8UgLM0FGVfUjxR8Q1VHEbRYRaG8CfBUGVIF2fFVb230LdbtcE";    // Đổi Gscript ID theo của bạn
int count = 0;  //bộ đếm stt

char light_array[7];
void setup() {
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

float getLightPercentage(void)
{
  int ldrRawVal;
  float percentage;
  ldrRawVal = analogRead(LDR_PIN);    
  percentage = ((float)((ldrRawVal*100)/4096));
  return percentage;
}

void loop() {
    float lightpercentage = getLightPercentage();
    String lightPer_s(lightpercentage);
    
    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+ "stt=" + count + "&sensor=" + lightpercentage;
    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    //---------------------------------------------------------------------
    http.end();
  
  count++;
  delay(1000);
} 
