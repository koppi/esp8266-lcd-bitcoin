#include <ESP8266WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

const char* ssid = "HITRON-1B31";
const char* pass = "AE5YV9G2APOG";

LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, NEGATIVE);

WiFiClient client;

char server[] = "api.coindesk.com";

// delay between updates, in msec
const unsigned long postingInterval = 60L * 1000L;

// last time you connected to the server, in msec
unsigned long lastConnectionTime = 0;

String id;
String value;

void wlan_connect() {
  int retries = 0;

  WiFi.begin(ssid, pass);

  while ((WiFi.status() != WL_CONNECTED)) {
    retries++;
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("\nWiFi connected: "));
    Serial.println(WiFi.localIP());
  }
}

void setup() {
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CoinDesk Bitcoin");
  lcd.setCursor(0,1);
  lcd.print("loading..");

  ESP.wdtDisable();

  Serial.begin(9600);
  delay(50);

  wlan_connect();
}

int check_connect = 0;

void httpRequest() {
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    client.println("GET /v1/bpi/currentprice.json HTTP/1.1");
    client.println("Host: api.coindesk.com");
    client.println("User-Agent: ESP8266/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void loop() {
  int cnt;
  
  if (cnt++ == 10000) {
    cnt = 0;
    if (check_connect++ == 50) {
      check_connect = 0;
      if (WiFi.status() != WL_CONNECTED) {
        wlan_connect();
      }
    }
  }
  
  /*
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }*/

  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
    unsigned int i = 0; //timeout counter
    int n = 1; // char counter
    char json[500] ="{";
  
    while (!client.find("\"EUR\":{")){}

    while (i<60000) {
      if (client.available()) {
        char c = client.read();
        json[n]=c;
        if (c=='}') break;
        n++;
        i=0;
      }
      i++;
    }
  
    Serial.println(json);
    
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    
    id = root["code"];
    value = root["rate"];
  
    Serial.print(id);
    Serial.print("= ");
    Serial.println(value);
    Serial.println();
  
    lcd.setCursor(0,1);
    lcd.print(value);
    lcd.print(" ");
    lcd.print(id);
    lcd.print("  ");

    id="";
    value="";
  }
}
