#include "DHT.h"
#include <ESP32Servo.h>

Servo myservo;

#define DHTTYPE DHT11

#define timeSeconds 10 //time in second to check how long set PIR_LED to HIGH

const int rain_sensor = 35;
const int dht_sensor = 19;
const int sound_sensor = 34;
const int servo_pin = 14;
const int pir_sensor = 32;
const int light_sensor = 39;
const int led_sound = 25;  //green
const int led_pir = 26; //yellow
const int led_light = 27; //red 

DHT dht(dht_sensor, DHTTYPE);

float humidity;
float temperature;
int val_rain;
int val_sound;
int val_light;
bool flg_sound = false;

unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;
  
void IRAM_ATTR detectsMovement() {
  digitalWrite(led_pir, HIGH);
  startTimer = true;
  lastTrigger = millis(); //บันทึกเวลาปัจจุบัน
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  myservo.attach(servo_pin, 500, 2500);
  myservo.write(90);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(pir_sensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(pir_sensor), detectsMovement, RISING);

  pinMode(led_sound, OUTPUT);
  pinMode(led_pir, OUTPUT);
  pinMode(led_light, OUTPUT);
}

void loop() {
  //////////////////////////////////humidity & temperature///////////////////////////
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Serial.print(F("Humidity: "));
  // Serial.print(humidity);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(temperature);
  // Serial.println(F("°C "));
  //////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////rain//////////////////////////////////////////////////
  val_rain = analogRead(rain_sensor);
  // Serial.print("rain value : ");
  // Serial.println(val_rain);
  if (val_rain < 3900) {
    myservo.write(0);
  } else if (val_rain > 4000) {
    myservo.write(90);
  }
  ///////////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////sound sensor///////////////////////////////
  val_sound = analogRead(sound_sensor);
  delay(100);
  // Serial.print("sound value : ");
  // Serial.print(val_sound);
  // Serial.print("     flg_value : ");
  // Serial.println(flg_sound);
  if (val_sound > 2500) {
    flg_sound = !flg_sound;
  }
  if (flg_sound) {
    digitalWrite(led_sound, HIGH);
  } else {
    digitalWrite(led_sound, LOW);
  }
  //////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////light sensor///////////////////////////////////
  val_light = analogRead(light_sensor);
  //Serial.print("light value : ");
  //Serial.println(val_light);
  if (val_light < 3000) {
    digitalWrite(led_light, LOW);
  } else if (val_light > 3500) {
    digitalWrite(led_light, HIGH);
  }
  ////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////pir (motion)/////////////////////////////////////
  // Current time
  now = millis();
  if((digitalRead(led_pir) == HIGH) && (motion == false)) {
    Serial.println("MOTION DETECTED!!!");
    motion = true;
  }
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(led_pir, LOW);
    startTimer = false;
    motion = false;
  }

  ////////////////////////////////////////////////////////////////////////////////
}