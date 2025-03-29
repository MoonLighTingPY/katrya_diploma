#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Налаштування пінів.
// На Arduino Uno треба буде використати інші піни та замінити їх тут
const int pirPin = 12;    // PIR сенсор руху (середній пін на сенсорі)
const int ledPin = 32;    // LED (діод, довга ножка підключена до піна, коротка до землі)
const int micPin = 33;    // KY-038 мікрофон (підключено до D0, тобто цифровий вихід сенсора)
const int lcdsdaPin = 26; // SDA пін для LCD дисплею
const int lcdsclPin = 25; // SCL пін для LCD дисплею

bool soundDetected = false; // Boolean для визначення чи виявлено звук. True якщо виявлено, інакше false
bool movementDetected = false; // Те саме, але для руху
unsigned long soundDetectedTime = 0; // Час коли виявлено звук
String lastLcdMessage = ""; // Трекати яке останнє повідомлення було виведено на LCD


// Ініціалізувати LCD дисплей. I2C Адреса 0x27, 16 стовпців, 2 рядки
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Функція для оновлення LCD дисплею з новими станами руху та звуку. Приймає два параметри: стан руху та стан звуку
// Оновлює дисплей тільки якщо повідомлення змінилося, щоб уникнути мерцання на дисплеї
void updateLCD(const String& motionState, const String& soundState) {
  // Онолювати тільки якщо повідомлення змінилося. Якщо ні, пропустити
  String newLcdMessage = motionState + "\n" + soundState;
  if (newLcdMessage != lastLcdMessage) {
    lcd.clear();
    lcd.setCursor(0, 0); // Почати з першого рядка
    lcd.print(motionState); // Вивести стан руху на першому рядку
    lcd.setCursor(0, 1); // Почати з другого рядка
    lcd.print(soundState);  // Вивести стан звуку на другому рядку
    lastLcdMessage = newLcdMessage; // Оновити останнє повідомлення, щоб не виводити його знову, якщо нічого не змінилося
  }
}

// Функція для зчитування стану PIR сенсора. Boolean, повертає true якщо рух виявлено, інакше false.
bool readPirSensor() {
  int currentPirState = digitalRead(pirPin);

  // Якщо PIR сенсор руху виявив рух, оновити стан руху (movementDetected)
  if (currentPirState == HIGH) {
    movementDetected = true;
  } else if (currentPirState == LOW) {
    movementDetected = false;
  }

  return movementDetected; // Повернути стан руху (true якщо виявлено, інакше false)
}

// Функція для зчитування стану мікрофону. Boolean, повертає true якщо звук виявлено, інакше false.
// Затримка 5 секунд після виявлення звуку, щоб відсіяти шум. Затримка на millis() а не delay() щоб не блокувати виконання коду
bool readMicSensor() {
  int micState = digitalRead(micPin);

  // Якщо мікрофон виявив звук, оновити стан звуку (soundDetected)
  if (micState == HIGH) {
    soundDetected = true;
    soundDetectedTime = millis(); // Запам'ятати час коли виявлено звук
  // Затримка щоб встигнути побачити зміну стану на LCD. (інакше відразу скаже знову "No Sound")
  // Якщо звук було виявлено більше 5 секунд тому, встановити стан звуку на false
  } else if (soundDetected && millis() - soundDetectedTime > 5000) {
    soundDetected = false;
  }

  return soundDetected; // Повернути стан звуку (true якщо виявлено, інакше false)
}

// Функція setup() викликається один раз при запуску програми, тобто коли мікроконтролер тільки включився, або перезапустився
void setup() {
  // Почати Serial
  Serial.begin(115200);

  // Ініціалізувати I2C для LCD дисплею
  Wire.begin(lcdsdaPin, lcdsclPin);

  // Ініціалізувати піни
  pinMode(pirPin, INPUT);  // PIR сенсор руху, вхід
  pinMode(ledPin, OUTPUT); // LED діод, вихід
  pinMode(micPin, INPUT);  // KY-038 мікрофон, вхід

  // Ініціалізувати LCD
  lcd.begin(16, 2);  // Встановити розмір LCD (16 стовпців, 2 рядки)
  lcd.setBacklight(true);  // Включити дисплей
  lcd.clear();  // Очистити дисплей(на випадок кщо щось вже виведено)

  // Вивести повідомлення про ініціалізацію 
  updateLCD("System Initialized", "No Sound");
  Serial.println("System Initialized"); // Вивести повідомлення в Serial
  delay(2000); // Затримка 2 секунди, щоб встигнути побачити повідомлення ініціалізації на дислеї
}

// Функція loop() викликається постійно по колу після того як виконано функцію setup()
void loop() {
  // Прочитати стан PIR сенсора руху
  bool motionDetected = readPirSensor();

  // Прочитати стан мікрофону
  bool soundDetected = readMicSensor();

  // Задати повідомлення для стану руху
  // Якщо виявлено рух, вивести "Motion Detected!", інакше "No Motion"
  String motionState = motionDetected ? "Motion Detected!" : "No Motion";

  // Включити LED якщо виявлено рух, вимкнути якщо ні
  // Просто подаєься напруга на пін: якщо HIGH - світиться, якщо LOW то ні
  digitalWrite(ledPin, motionDetected ? HIGH : LOW);

  // Задати повідомлення для стану звуку
  // Якщо виявлено звук, вивести "Sound Detected!", інакше "No Sound"
  String soundState = soundDetected ? "Sound Detected!" : "No Sound";

  // Оновити LCD дисплей з новими станами руху та звуку
  updateLCD(motionState, soundState);
}