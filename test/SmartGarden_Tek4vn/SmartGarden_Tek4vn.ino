#define BLYNK_TEMPLATE_ID           "TMPLC2bbysVN"      
#define BLYNK_DEVICE_NAME           "SmartGarden"               //paste phần vừa copy
#define BLYNK_AUTH_TOKEN            "Lcq_f2OnmEyHYmOZiV9WYErnpQCPijGA"   // các thông tin để xác minh trong blynk.io

#define BLYNK_PRINT Serial

#include <WiFi.h>           //khai báo các thư viện cần thiết
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>    // tải thêm trong Sketch ở Arduino IDE
#include <DHT.h>            // tải thêm trong Sketch ở Arduino IDE

char auth[] = BLYNK_AUTH_TOKEN;

// Thông tin WIFI của bạn
// Thay đổi trong ngoặc kép ""
char ssid[] = "ESP32";
char pass[] = "hoilamgi12";

#define DHTPIN 21          // Khai báo những chân kết nối với cảm biến và Relay
#define DHTTYPE DHT11
#define SOILMOISTURE_PIN 35     //chân độ ẩm đất
#define RELAY_PIN 23     //relay pin

DHT dht(DHTPIN, DHTTYPE);       //khai báo chân và loại dht
BlynkTimer timer;

float t =0;           //khai báo biến nhiệt độ toàn cục
int h = 0;            //khai báo biến độ ẩm đất toàn cục
int manual_button=0;   //khai báo gtri nút chỉnh chế độ
int buttonState=0;      //khai báo giá trị nút bơm tay

void sendSensor()
{
  int hu = analogRead(SOILMOISTURE_PIN);    //đọc giá trị analog từ cảm biến độ ẩm đất, mng chú ý cắm chân 3v3 cho cảm biến nha
  h = map(hu,0,4095,100,0);  //đổi giá trị sang từ 0 -> 100
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  Serial.print(hu);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Không gửi nhiều hơn 10 gtri trong 1s
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V0, t);
}

void setup()
{
  pinMode(SOILMOISTURE_PIN,INPUT);
  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN,HIGH);
  // Debug console
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  // Setup a function to be called every second
  timer.setInterval(500L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
  Mode();
}
BLYNK_WRITE(V4){          //đọc gtri nút từ app blynk
  manual_button =  param.asInt();
  Serial.println(manual_button);
}
WidgetLED led_state(V3);
BLYNK_WRITE(V2){                      //auto hoac manual
  buttonState = param.asInt();
  Serial.println(buttonState);
}

void Mode(){                    //chọn chế độ làm việc là auto hoặc manual
  if (buttonState == 1) {
    if (h <=42 && t <=35){            // thay đổi thông số mà bạn mong muốn hoạt động auto ở đây
      digitalWrite(RELAY_PIN, LOW);
      led_state.on();
    }else if ( h >42) {               //... cả chỗ này nữa
      digitalWrite(RELAY_PIN, HIGH);
      led_state.off();
    }
  }else if (buttonState == 0){
      if (manual_button == 1){
      digitalWrite(RELAY_PIN, LOW);
      led_state.on();

    }else {
      digitalWrite(RELAY_PIN, HIGH);
      led_state.off();
    }
  }
}
