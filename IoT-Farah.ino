#include <OneWire.h>
#include <DallasTemperature.h>

// Pin definitions
#define ONE_WIRE_BUS 6  // Pin connected to DS18B20
#define TRIG_PIN 5       // Pin connected to HC-SR04 TRIG
#define ECHO_PIN 18       // Pin connected to HC-SR04 ECHO
#define PH_PIN 7         // Pin connected to potentiometer for pH simulation
#define TDS_PIN 15       // Pin connected to potentiometer for TDS simulation
#define FLOW_PIN 16      // Pin connected to potentiometer for flow simulation
#define BUZZER_PIN 8     // Pin connected to Buzzer

// OneWire and DallasTemperature objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Fuzzy logic thresholds
#define PH_LOW 5.5
#define PH_HIGH 7.0

#define TDS_LOW 100
#define TDS_HIGH 2000

#define FLOW_LOW 2
#define FLOW_HIGH 5

#define TEMP_LOW 25
#define TEMP_HIGH 27

#define LEVEL_LOW 50
#define LEVEL_HIGH 150

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize DS18B20
  sensors.begin();

  // Initialize HC-SR04 pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize sensor pins
  pinMode(PH_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);
  pinMode(FLOW_PIN, INPUT);

  // Initialize Buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
}

String fuzzyLogic(float value, float low, float high) {
  if (value < low) {
    return "Low";
  } else if (value > high) {
    return "High";
  } else {
    return "Normal";
  }
}

void loop() {
  // Read temperature from DS18B20
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0); // Get temperature in Celsius

  // Read level from HC-SR04
  long duration;
  float distance;

  // Send a 10us pulse to trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pin
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in cm (speed of sound is ~343 m/s)
  distance = duration * 0.034 / 2;

  // Read pH value from potentiometer
  int phValueRaw = analogRead(PH_PIN);
  float phValue = map(phValueRaw, 0, 1023, 0, 7); // Map to pH range (0-7)

  // Read TDS value from potentiometer
  int tdsValueRaw = analogRead(TDS_PIN);
  float tdsValue = map(tdsValueRaw, 0, 1023, 0, 2000); // Map to TDS range (0-2000 ppm)

  // Read flow value from potentiometer
  int flowValueRaw = analogRead(FLOW_PIN);
  float flowValue = map(flowValueRaw, 0, 1023, 0, 5); // Map to flow range (0-5 L/Min)

  // Fuzzy logic evaluations
  String phFuzzy = fuzzyLogic(phValue, PH_LOW, PH_HIGH);
  String tdsFuzzy = fuzzyLogic(tdsValue, TDS_LOW, TDS_HIGH);
  String flowFuzzy = fuzzyLogic(flowValue, FLOW_LOW, FLOW_HIGH);
  String tempFuzzy = fuzzyLogic(temperatureC, TEMP_LOW, TEMP_HIGH);
  String levelFuzzy = fuzzyLogic(distance, LEVEL_LOW, LEVEL_HIGH);

  // Check for alert conditions and activate buzzer
  if (phFuzzy == "Low" || phFuzzy == "High" || tdsFuzzy == "High" || flowFuzzy == "High" || tempFuzzy == "High" || tempFuzzy == "Low" || levelFuzzy == "Low" || levelFuzzy == "High") {
    digitalWrite(BUZZER_PIN, HIGH); // Activate buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Deactivate buzzer
  }

  Serial.print("Temperature (C): ");
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: DS18B20 not connected");
  } else {
    Serial.print(temperatureC);
    Serial.print(" (" + tempFuzzy + ")\n");
  }

  Serial.print("Level (cm): ");
  if (distance < 0 || distance > 150) { // Assuming valid range is 0-150 cm
    Serial.println("Out of range");
  } else {
    Serial.print(distance);
    Serial.print(" (" + levelFuzzy + ")\n");
  }

  Serial.print("pH: ");
  Serial.print(phValue);
  Serial.print(" (" + phFuzzy + ")\n");

  Serial.print("TDS (ppm): ");
  Serial.print(tdsValue);
  Serial.print(" (" + tdsFuzzy + ")\n");

  Serial.print("Flow (L/Min): ");
  Serial.print(flowValue);
  Serial.print(" (" + flowFuzzy + ")\n");

  delay(1000);
}