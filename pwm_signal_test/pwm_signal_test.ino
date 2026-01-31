#define THROTTLE 2

volatile long unsigned int signalStart = 0;
volatile long unsigned int signalEnd = 0;
volatile long unsigned int signalLength = 0;

void pwm_reading_interrupt() {
  int isHigh = digitalRead(THROTTLE);
  if (isHigh) {
    signalStart = micros();
  } else {
    signalEnd = micros();
    update(); 
  }
}

void update() {
  signalLength = signalEnd - signalStart;
  Serial.print(signalLength);

  int pwmValue = map(signalLength, 1000, 2000, 0, 255);
  int dutyValue = constrain(pwmValue, 0, 255);
  
  Serial.print(" ");
  Serial.println(dutyValue);
}

void setup() {
  Serial.begin(9600);
  pinMode(THROTTLE, INPUT);
  attachInterrupt(digitalPinToInterrupt(THROTTLE), pwm_reading_interrupt, CHANGE);
}

void loop() {
  // Your main code here
}
