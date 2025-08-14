// AQI Monitoring System Code
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHT_PIN 7
#define MQ2_PIN A2
#define DUST_PIN A0
#define DUST_LED_PIN 3

#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(DUST_LED_PIN, OUTPUT);
  digitalWrite(DUST_LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("AQI Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read sensors
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gas_val = analogRead(MQ2_PIN);

  // Pulse the dust sensor's LED to get a reading
  digitalWrite(DUST_LED_PIN, HIGH);
  delayMicroseconds(280);
  int dust_val = analogRead(DUST_PIN);
  delayMicroseconds(40);
  digitalWrite(DUST_LED_PIN, LOW);

  // Convert sensor values to concentrations (simple mapping, needs calibration)
  float gas_conc = map(gas_val, 0, 1023, 0, 500);
  float dust_conc = map(dust_val, 0, 1023, 0, 500);

  // Calculate AQI (use the worse of the two pollutants)
  int aqi_gas = getAQI(gas_conc);
  int aqi_dust = getAQI(dust_conc);
  int final_aqi = max(aqi_gas, aqi_dust);

  // Display results on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AQI: " + String(final_aqi));
  lcd.setCursor(8, 0);
  lcd.print(getStatus(final_aqi));
  lcd.setCursor(0, 1);
  lcd.print("T:" + String(temp, 1) + "C H:" + String(humidity, 0) + "%");

  // Print to serial for debugging
  Serial.print("Temp: " + String(temp) + "C, ");
  Serial.print("Final AQI: " + String(final_aqi));
  Serial.println();

  delay(2000);
}

int getAQI(float concentration) {
  if (concentration <= 50) return map(concentration, 0, 50, 0, 50);
  if (concentration <= 100) return map(concentration, 51, 100, 51, 100);
  if (concentration <= 150) return map(concentration, 101, 150, 101, 150);
  if (concentration <= 200) return map(concentration, 151, 200, 151, 200);
  if (concentration <= 300) return map(concentration, 201, 300, 201, 300);
  return map(concentration, 301, 500, 301, 500);
}

String getStatus(int aqi) {
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Moderate";
  if (aqi <= 150) return "UnhealthyS";
  if (aqi <= 200) return "Unhealthy";
  if (aqi <= 300) return "Very Unh.";
  return "Hazardous";
}
