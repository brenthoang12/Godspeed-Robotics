#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 150  // Pulse for 0° (adjust as needed)
#define SERVOMAX 600  // Pulse for 180° (adjust as needed)

const int THRESHOLD = 10;

// input
const int buttonPin = 2; 
const int xPin = A0; 
const int yPin = A1; 

// x axis movement
const int xCenter = 510;
int xVal;
int xAngle;
int xPulse;

// y axis movement
const int yCenter = 536;
int yVal;
int yAngle;
int yPulse;

// lid movement
int switchState;
int centerDeviate;

// right lid movement
int right_upLidAngle;
int right_lowLidAngle;

// left lid movement
int left_upLidAngle;
int left_lowLidAngle;

int degreeToPulse(int degree) {
  return map(degree, 0, 180, SERVOMIN, SERVOMAX);
}

void close_eye() {
  pwm.setPWM(2, 0, degreeToPulse(115));
  pwm.setPWM(3, 0, degreeToPulse(65));
  pwm.setPWM(4, 0, degreeToPulse(60));
  pwm.setPWM(5, 0, degreeToPulse(115));
}

void reset() {
  pwm.setPWM(0, 0, degreeToPulse(90)); // range: 60 - 120 degree left - right
  pwm.setPWM(1, 0, degreeToPulse(90)); // range: 70 - 110 degree down - up
  pwm.setPWM(2, 0, degreeToPulse(90));
  pwm.setPWM(3, 0, degreeToPulse(90));
  pwm.setPWM(4, 0, degreeToPulse(85));
  pwm.setPWM(5, 0, degreeToPulse(90));
  delay(500);
}

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);

  pwm.begin();
  pwm.setPWMFreq(50);

  reset();
}

void loop() {
  // read input
  switchState = digitalRead(buttonPin);
  xVal = analogRead(xPin);
  yVal = analogRead(yPin);

  // eye balls 
  xAngle = (abs(xVal - xCenter) > THRESHOLD) ? map(xVal, 0, 1023, 60, 120) : 90;
  yAngle = (abs(yVal - yCenter) > THRESHOLD) ? map(yVal, 0, 1023, 120, 60) : 90;
  xPulse = degreeToPulse(xAngle);
  yPulse = degreeToPulse(yAngle);

  // eye lids 
  centerDeviate = yAngle - 90;
  right_upLidAngle = constrain(90-centerDeviate, 70, 110);
  right_lowLidAngle = constrain(90-centerDeviate, 70, 110);
  left_upLidAngle = constrain(82+centerDeviate, 70, 110);
  left_lowLidAngle = constrain(90+centerDeviate, 70, 110);

  //movement 
  pwm.setPWM(0, 0, xPulse);
  pwm.setPWM(1, 0, yPulse);

  if (switchState == HIGH) {
    pwm.setPWM(2, 0, degreeToPulse(right_upLidAngle));  
    pwm.setPWM(3, 0, degreeToPulse(right_lowLidAngle));  
    pwm.setPWM(4, 0, degreeToPulse(left_upLidAngle));  
    pwm.setPWM(5, 0, degreeToPulse(left_lowLidAngle)); 
  } else if (switchState == LOW) {
    close_eye();
  }
  
  delay(50);
}