
#define a1 2
#define a2 4
#define b1 5
#define b2 6
#define speed_A 3
#define speed_B 11

const int sensorPins[5] = {12, 13, 10, 9, 8};
const int weights[5] = {-4, -2, 0, 2, 4};

// Control parameters
const int LINE_LEVEL = LOW;
const int BASE = 150;
const float KP = 18.0;
const float KD = 25.0;

const int BRIDGE_BOOST = 30;
const unsigned long LOST_STRAIGHT_MS = 250;
const int SEARCH_SPEED = 120;
const unsigned long FINISH_MS = 300;

// State variables
float lastError = 0;
bool allBlack = false;
unsigned long blackStart = 0;
bool lost = false;
unsigned long lostStart = 0;

// Motor control
void motorA(int v) {
  v = constrain(v, -255, 255);
  digitalWrite(a1, v >= 0 ? LOW : HIGH);
  digitalWrite(a2, v >= 0 ? HIGH : LOW);
  analogWrite(speed_A, abs(v));
}

void motorB(int v) {
  v = constrain(v, -255, 255);
  digitalWrite(b1, v >= 0 ? HIGH : LOW);
  digitalWrite(b2, v >= 0 ? LOW : HIGH);
  analogWrite(speed_B, abs(v));
}

void stopMotors() {
  digitalWrite(a1, LOW);
  digitalWrite(a2, LOW);
  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
  analogWrite(speed_A, 0);
  analogWrite(speed_B, 0);
}

void setup() {
  int outputs[] = {a1, a2, b1, b2, speed_A, speed_B};

  for (int i = 0; i < 6; i++) {
    pinMode(outputs[i], OUTPUT);
  }

  for (int i = 0; i < 5; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  stopMotors();
}

void loop() {
  int sum = 0;
  int cnt = 0;

  for (int i = 0; i < 5; i++) {
    if (digitalRead(sensorPins[i]) == LINE_LEVEL) {
      sum += weights[i];
      cnt++;
    }
  }

  // Intersection or finish line
  if (cnt == 5) {
    lost = false;

    if (!allBlack) {
      allBlack = true;
      blackStart = millis();
    }

    if (millis() - blackStart >= FINISH_MS) {
      stopMotors();
      while (true) {}
    }

    motorA(BASE);
    motorB(BASE);
    return;
  }

  allBlack = false;

  // Line lost recovery
  if (cnt == 0) {
    if (!lost) {
      lost = true;
      lostStart = millis();
    }

    if (millis() - lostStart < LOST_STRAIGHT_MS) {
      motorA(BASE + BRIDGE_BOOST);
      motorB(BASE + BRIDGE_BOOST);
    } else if (lastError < 0) {
      motorA(-SEARCH_SPEED);
      motorB(SEARCH_SPEED);
    } else {
      motorA(SEARCH_SPEED);
      motorB(-SEARCH_SPEED);
    }

    return;
  }

  lost = false;

  // PD line following
  float error = (float)sum / cnt;
  float correction = KP * error + KD * (error - lastError);

  lastError = error;

  motorA(BASE + (int)correction);
  motorB(BASE - (int)correction);

  delay(5);
}
