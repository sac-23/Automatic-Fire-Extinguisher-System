#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "Your_SSID";
const char* password = "Your_Password";

#define FlameSensor 34 
#define RelayModule 4 
#define Buzzer 15 

long myChannelNumber = 123456;  // Replace with your ThingSpeak Channel Number
const char* myWriteAPIKey = "Your_Write_API_Key";

const int flameThreshold = 300;
int waterpump, fireDetected; 

WiFiClient client; 

void WiFiSetup() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi...");
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
        
        if (millis() - startTime > 20000) {  // Timeout after 20 seconds
            Serial.println("\nWiFi connection failed! Restarting...");
            ESP.restart();
        }
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    
    pinMode(FlameSensor, INPUT);
    pinMode(RelayModule, OUTPUT);
    pinMode(Buzzer, OUTPUT);

    digitalWrite(RelayModule, LOW); // Initially turn off the relay
    digitalWrite(Buzzer, LOW); // Initially turn off the buzzer

    WiFiSetup();
    
    ThingSpeak.begin(client); 
}

void loop() {
    int flameValue = analogRead(FlameSensor);
    Serial.print("Flame Sensor Value: ");
    Serial.println(flameValue);

    if (flameValue < flameThreshold) { 
        Serial.println("Fire Detected! Activating extinguisher.");
        fireDetected = 1;

        digitalWrite(RelayModule, HIGH); // Activate the relay (water pump ON)
        waterpump = 1;
        digitalWrite(Buzzer, HIGH); // Activate the buzzer
        delay(2000);
        digitalWrite(Buzzer, LOW); // Turn off the buzzer
    } 
    else {
        Serial.println("No fire detected. System is monitoring.");
        digitalWrite(RelayModule, LOW); // Turn off the relay (water pump OFF)
        fireDetected = 0;
        waterpump = 0;
    }

    sendDataToThingSpeak(fireDetected, flameValue, waterpump);
    delay(2000); // Reduced delay for more frequent updates
}

void sendDataToThingSpeak(int fireDetected, int flameIntensity, int waterpumpStatus) {
    ThingSpeak.setField(1, fireDetected);
    ThingSpeak.setField(2, flameIntensity);
    ThingSpeak.setField(3, waterpumpStatus);

    int responseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (responseCode == 200) {
        Serial.println(" Data sent to ThingSpeak successfully.");
    } else {
        Serial.print("Error sending data to ThingSpeak. Response code: ");
        Serial.println(responseCode);
    }
}
