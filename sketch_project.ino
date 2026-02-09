// ДРАЙВЕР 1: МОТОРЫ A и D
const int STBY_1 = 2;
const int PWMA_1 = 3;      // Мотор A (задний левый)
const int AIN2_1 = 4;
const int AIN1_1 = 5;
const int PWMB_1 = 6;      // Мотор D (задний правый)
const int BIN1_1 = 7;
const int BIN2_1 = 8;

// ДРАЙВЕР 2: МОТОРЫ B и C
const int STBY_2 = 9;
const int PWMA_2 = 10;     // Мотор C (передний правый)
const int AIN2_2 = 11;
const int AIN1_2 = 12;
const int PWMB_2 = 13;     // Мотор B - ШИМ на 13
const int BIN1_2 = A4;     // Мотор B - направление на A4 (вместо A1)
const int BIN2_2 = A5;     // Мотор B - направление на A5 (вместо A2)

// УЛЬТРАЗВУКОВОЙ ДАТЧИК - TRIG на A3, ECHO на A2
const int TRIG_PIN = A3;   // Ультразвук TRIG остается на A3
const int ECHO_PIN = A2;   // Ультразвук ECHO на A2

unsigned long t = 0;
byte state = 0;

// ИЗМЕНЕННЫЕ НАСТРОЙКИ
int turnTime = 440;        // Уменьшили на 50% (было 880)
int normalSpeed = 150;     // Увеличили на 25% (было 120, 120 × 1.25 = 150)
int reverseSpeed = 125;    // Увеличили на 25% (было 100, 100 × 1.25 = 125)
int turnSpeed = 242;       // Уменьшили на 5% (было 255)
float obstacleDistance = 30;
int stopDelay = 6000;      // Время остановки 6 секунд (было 1 секунда)

void setup() {
  // ДРАЙВЕР 1
  pinMode(STBY_1, OUTPUT);
  pinMode(PWMA_1, OUTPUT);
  pinMode(AIN1_1, OUTPUT);
  pinMode(AIN2_1, OUTPUT);
  pinMode(PWMB_1, OUTPUT);
  pinMode(BIN1_1, OUTPUT);
  pinMode(BIN2_1, OUTPUT);
  
  // ДРАЙВЕР 2
  pinMode(STBY_2, OUTPUT);
  pinMode(PWMA_2, OUTPUT);
  pinMode(AIN1_2, OUTPUT);
  pinMode(AIN2_2, OUTPUT);
  pinMode(PWMB_2, OUTPUT);
  pinMode(BIN1_2, OUTPUT);  // A4 как OUTPUT
  pinMode(BIN2_2, OUTPUT);  // A5 как OUTPUT
  
  // УЛЬТРАЗВУК
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  digitalWrite(STBY_1, HIGH);
  digitalWrite(STBY_2, HIGH);
  
  Serial.begin(9600);
  Serial.println("Start - новые настройки");
  Serial.print("normalSpeed: "); Serial.println(normalSpeed);
  Serial.print("reverseSpeed: "); Serial.println(reverseSpeed);
  Serial.print("turnSpeed: "); Serial.println(turnSpeed);
  Serial.print("turnTime: "); Serial.print(turnTime); Serial.println(" мс");
  Serial.print("stopDelay: "); Serial.print(stopDelay); Serial.println(" мс");
}

void loop() {
  float distance = getDist();
  
  switch(state) {
    case 0: // ВПЕРЕД
      if(distance < obstacleDistance) {
        stopAll();
        state = 1;
        t = millis();
        Serial.print("STOP - препятствие на ");
        Serial.print(distance);
        Serial.println(" см");
        Serial.println("Остановка на 6 секунд...");
      } else {
        // ВСЕ 4 МОТОРА ВПЕРЕД
        
        // Мотор A вперед
        digitalWrite(AIN1_1, HIGH);
        digitalWrite(AIN2_1, LOW);
        analogWrite(PWMA_1, normalSpeed);
        
        // Мотор D вперед
        digitalWrite(BIN1_1, LOW);
        digitalWrite(BIN2_1, HIGH);
        analogWrite(PWMB_1, normalSpeed);
        
        // Мотор C вперед
        digitalWrite(AIN1_2, HIGH);
        digitalWrite(AIN2_2, LOW);
        analogWrite(PWMA_2, normalSpeed);
        
        // Мотор B вперед - используем LOW/HIGH (то, что работает)
        digitalWrite(BIN1_2, LOW);    // Для мотора B LOW/HIGH работает
        digitalWrite(BIN2_2, HIGH);
        analogWrite(PWMB_2, normalSpeed);
        
        Serial.println("Движение ВПЕРЕД");
      }
      break;
      
    case 1: // СТОИМ 6 СЕКУНД
      if(millis() - t >= stopDelay) {  // 6 секунд вместо 1
        state = 2;
        t = millis();
        Serial.println("Движение НАЗАД 1.5 секунды");
        
        // ВСЕ 4 МОТОРА НАЗАД
        
        // Мотор A назад
        digitalWrite(AIN1_1, LOW);
        digitalWrite(AIN2_1, HIGH);
        analogWrite(PWMA_1, reverseSpeed);
        
        // Мотор D назад
        digitalWrite(BIN1_1, HIGH);
        digitalWrite(BIN2_1, LOW);
        analogWrite(PWMB_1, reverseSpeed);
        
        // Мотор C назад
        digitalWrite(AIN1_2, LOW);
        digitalWrite(AIN2_2, HIGH);
        analogWrite(PWMA_2, reverseSpeed);
        
        // Мотор B назад - используем HIGH/LOW (противоположно)
        digitalWrite(BIN1_2, HIGH);   // Для мотора B HIGH/LOW
        digitalWrite(BIN2_2, LOW);
        analogWrite(PWMB_2, reverseSpeed);
      }
      break;
      
    case 2: // ЕДЕМ НАЗАД
      if(millis() - t >= 1500) {
        stopAll();
        delay(300);
        state = 3;
        t = millis();
        Serial.println("ПОВОРОТ НАЛЕВО НА МЕСТЕ (440 мс)");
        
        // ПОВОРОТ: A и B НАЗАД, C и D ВПЕРЕД
        // Но для мотора B "назад" это HIGH/LOW
        
        // Мотор A НАЗАД
        digitalWrite(AIN1_1, LOW);
        digitalWrite(AIN2_1, HIGH);
        analogWrite(PWMA_1, turnSpeed);
        
        // Мотор B НАЗАД
        digitalWrite(BIN1_2, HIGH);   // НАЗАД для B
        digitalWrite(BIN2_2, LOW);
        analogWrite(PWMB_2, turnSpeed);
        
        // Мотор C ВПЕРЕД
        digitalWrite(AIN1_2, HIGH);
        digitalWrite(AIN2_2, LOW);
        analogWrite(PWMA_2, turnSpeed);
        
        // Мотор D ВПЕРЕД
        digitalWrite(BIN1_1, LOW);
        digitalWrite(BIN2_1, HIGH);
        analogWrite(PWMB_1, turnSpeed);
      }
      break;
      
    case 3: // ПОВОРАЧИВАЕМ
      if(millis() - t >= turnTime) {
        stopAll();
        delay(300);
        state = 0;
        Serial.println("СНОВА ВПЕРЕД");
      }
      break;
  }
  
  delay(50);
}

void stopAll() {
  analogWrite(PWMA_1, 0);
  analogWrite(PWMB_1, 0);
  analogWrite(PWMA_2, 0);
  analogWrite(PWMB_2, 0);
}

float getDist() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long d = pulseIn(ECHO_PIN, HIGH, 40000);
  if(d == 0) return 300;
  
  float cm = d * 0.034 / 2;
  if(cm < 2 || cm > 300) return 300;
  
  return cm;
}
