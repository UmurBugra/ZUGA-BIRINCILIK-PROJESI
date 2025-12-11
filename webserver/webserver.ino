#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

const char* ssid = "FallGuard_Medical";
WebServer server(80);

String durum_mesaji = "GUVENLI";
String arka_plan_renk = "green";
unsigned long son_alarm_zamani = 0;
String gelen_kanal = "YOK";

// ÜRETİCİ KODUNDAN ALINAN DOĞRU PİNLER
#define RX_PIN 26 
#define TX_PIN 25
#define LED_PIN 2 // Mavi LED

String getHTML() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"refresh\" content=\"2\">"; 
  html += "<style>body { font-family: Arial; text-align: center; margin: 0; padding-top: 50px; background-color: #f4f4f4; }";
  html += ".status { padding: 20px; font-size: 30px; font-weight: bold; color: white; border-radius: 5px; background-color: " + arka_plan_renk + "; }</style></head><body>";
  html += "<h1>SmartLife FallGuard</h1><p>Hasta Durumu:</p><div class=\"status\">" + durum_mesaji + "</div>";
  html += "<p>Baglanti: " + gelen_kanal + "</p>";
  html += "</body></html>";
  return html;
}

void handleRoot() { server.send(200, "text/html", getHTML()); }

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200); // Bilgisayar Hattı

  // STM32 Hattı (26 ve 25 Pini)
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); 

  WiFi.softAP(ssid);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Sistem Hazir. Jumper 1-2 Modunda Dinleniyor...");
}

void loop() {
  server.handleClient();

  // --- STM32'den Veri Geldi mi? ---
  if (Serial2.available()) {
    // 1. Veriyi Oku
    String gelen = Serial2.readStringUntil('\n');
    gelen.trim(); // Boşlukları temizle ("DUSME\n" -> "DUSME" olur)
    
    // 2. Debug Ekranına Yaz (PC'de gör)
    Serial.print("STM32: ");
    Serial.println(gelen);

    // 3. İçeriği Kontrol Et
    if (gelen.indexOf("DUSME") >= 0) {
      durum_mesaji = "DUSME TESPIT EDILDI!";
      arka_plan_renk = "red";
      son_alarm_zamani = millis();
      gelen_kanal = "STM32 (OK)";
      
      // LED Yak (Onay)
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
    }
  }

  // 15 saniye sonra sistemi sıfırla
  if (arka_plan_renk == "red" && (millis() - son_alarm_zamani > 15000)) {
    durum_mesaji = "GUVENLI";
    arka_plan_renk = "green";
    gelen_kanal = "Sistem Sifirlandi";
  }
}