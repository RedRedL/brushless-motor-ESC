
const int on_time = 25;
const int off_time = 150;
int bldc_state = 0;

// Define pinouts
const int A_driver_IN = 10;
const int A_driver_SD = 9;

const int B_driver_IN = 5;
const int B_driver_SD = 11;

const int C_driver_IN = 2;
const int C_driver_SD = 3;

void setup() {
  // Set driver pins to output
  pinMode(A_driver_IN, OUTPUT);
  pinMode(A_driver_SD, OUTPUT);

  pinMode(B_driver_IN, OUTPUT);
  pinMode(B_driver_SD, OUTPUT);

  pinMode(B_driver_IN, OUTPUT);
  pinMode(B_driver_SD, OUTPUT);

  // Default each input to low
  digitalWrite(A_driver_IN, LOW);
  digitalWrite(B_driver_IN, LOW);
  digitalWrite(C_driver_IN, LOW);

  // Ensure each capacitor is charged
  digitalWrite(A_driver_SD, HIGH);
  digitalWrite(B_driver_SD, HIGH);
  digitalWrite(C_driver_SD, HIGH);
  delay(20);
  digitalWrite(A_driver_SD, LOW);
  digitalWrite(B_driver_SD, LOW);
  digitalWrite(C_driver_SD, LOW);

}

void bldc_move(int bldc_state) {
  switch (bldc_state) {
    case 0: // Driver A is high, Driver B is low.
      // Set A High
      digitalWrite(A_driver_IN, HIGH);
      // Set B Low
      digitalWrite(B_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(A_driver_SD, HIGH);
      digitalWrite(B_driver_SD, HIGH);
      break;

    case 1: // Driver A is high, Driver C is low.
      // Set A High
      digitalWrite(A_driver_IN, HIGH);
      // Set C Low
      digitalWrite(C_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(A_driver_SD, HIGH);
      digitalWrite(C_driver_SD, HIGH);
      break;

    case 2: // Driver B is high, Driver C is low.
      // Set B High
      digitalWrite(B_driver_IN, HIGH);
      // Set C Low
      digitalWrite(C_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(B_driver_SD, HIGH);
      digitalWrite(C_driver_SD, HIGH);
      break;
    case 3: // Driver B is high, Driver A is low.
      // Set B High
      digitalWrite(B_driver_IN, HIGH);
      // Set A Low
      digitalWrite(A_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(B_driver_SD, HIGH);
      digitalWrite(A_driver_SD, HIGH);
      break;
    case 4: // Driver C is high, Driver A is low.
      // Set C High
      digitalWrite(C_driver_IN, HIGH);
      // Set A Low
      digitalWrite(A_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(C_driver_SD, HIGH);
      digitalWrite(A_driver_SD, HIGH);
      break;
    case 5: // Driver C is high, Driver B is low.
      // Set C High
      digitalWrite(C_driver_IN, HIGH);
      // Set B Low
      digitalWrite(B_driver_IN, LOW);

      // Activate used drivers
      digitalWrite(C_driver_SD, HIGH);
      digitalWrite(B_driver_SD, HIGH);
      break;
  }
}

void bldc_off() {
  // Deactivate each driver
  digitalWrite(A_driver_SD, LOW);
  digitalWrite(B_driver_SD, LOW);
  digitalWrite(C_driver_SD, LOW);

  // Default each input to low
  digitalWrite(A_driver_IN, LOW);
  digitalWrite(B_driver_IN, LOW);
  digitalWrite(C_driver_IN, LOW);
}


void loop() {
  bldc_move(bldc_state);
  delay(on_time);
  bldc_off();
  bldc_state++;
  bldc_state %= 6;
  delay(off_time);
}
