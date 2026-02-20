/*
Author: RedRedL
Creation Date: 13/01/2026
Version: 6.0
References:
  [https://docs.arduino.cc/retired/hacking/software/PortManipulation/](https://docs.arduino.cc/retired/hacking/software/PortManipulation/)
  [https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
  [https://docs.arduino.cc/language-reference/en/functions/math/map/](https://docs.arduino.cc/language-reference/en/functions/math/map/)
  [https://simple-circuit.com/arduino-brushless-dc-motor-controller/](https://simple-circuit.com/arduino-brushless-dc-motor-controller/)
  [https://www.youtube.com/watch?v=F8l9Z1HGYcU](https://www.youtube.com/watch?v=F8l9Z1HGYcU)
*/

#define THROTTLE 2

const byte MAX_PWM_DUTY = 255;
const byte MIN_PWM_DUTY = 50;
const byte DEACTIVATION_THRESHHOLD = 5;
volatile byte bldc_state = 0;

volatile bool motorActive = false;

volatile long unsigned int signalStart = 0;
volatile long unsigned int signalEnd = 0;
volatile long unsigned int signalLength = 0;
volatile byte pendingDutyValue = 50;

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

  if (signalLength < 900 || signalLength > 2100) {
    pendingDutyValue = 0;
    return;
  }

  int pwmValue = map(signalLength, 1000, 2000, 0, 255);
  pendingDutyValue = constrain(pwmValue, 0, 255);
  }

void adjust_pwm(int duty) {
  if (duty < MIN_PWM_DUTY)
    duty = MIN_PWM_DUTY;
  else if (duty > MAX_PWM_DUTY)
    duty = MAX_PWM_DUTY;
  OCR1A = duty;
  OCR2A = duty;
  OCR2B = duty;
}


void disable_driver_pins() {
  PORTB &= ~((1 << PB1) | (1 << PB2) | (1 << PB3));
  PORTD &= ~((1 << PD4) | (1 << PD3) | (1 << PD5));
  TCCR1A &= ~(1 << COM1A1);
  TCCR2A &= ~((1 << COM2A1) | (1 << COM2B1));
  }


void bldc_next(byte bldc_state) {
  switch(bldc_state){
    /*
    Driver A: SD -> TCCR1A::COM1A1 -> PORTB::PB1; IN -> PORTB::PB2
    Driver B: SD -> TCCR2A::COM2A1 -> PORTB::PB3; IN -> PORTD::PD5
    Driver C: SD -> TCCR2A::COM2B1 -> PORTD::PD3; IN -> PORTD::PD4
    */
    case 0: // Driver A is high, Driver B is low.
      disable_driver_pins();
      // Enable PWM on Driver A, set IN high
      TCCR1A |= (1 << COM1A1);
      PORTB |= (1 << PB2);  // IN high for Driver A
      PORTB |= (1 << PB3);  // SD high for Driver B (but PWM disabled)
      ADMUX = 6;
      ACSR |= (1 << ACIS1) | (1 << ACIS0);
      break;
    case 1: // Driver A is high, Driver C is low.
      disable_driver_pins();
      TCCR1A |= (1 << COM1A1);
      PORTB |= (1 << PB2);  // IN high for Driver A
      PORTD |= (1 << PD3);  // SD high for Driver C
      ADMUX = 5;
      ACSR &= ~(1 << ACIS0);
      break;
    case 2: // Driver B is high, Driver C is low.
      disable_driver_pins();
      TCCR2A |= (1 << COM2A1);
      PORTD |= (1 << PD5);  // IN high for Driver B
      PORTD |= (1 << PD3);  // SD high for Driver C
      ADMUX = 4;
      ACSR |= (1 << ACIS1) | (1 << ACIS0);
      break;
    case 3: // Driver B is high, Driver A is low.
      disable_driver_pins();
      TCCR2A |= (1 << COM2A1);
      PORTD |= (1 << PD5);  // IN high for Driver B
      PORTB |= (1 << PB1);  // SD high for Driver A
      ADMUX = 6;
      ACSR &= ~(1 << ACIS0);
      break;
    case 4: // Driver C is high, Driver A is low.
      disable_driver_pins();
      TCCR2A |= (1 << COM2B1);
      PORTD |= (1 << PD4);  // IN high for Driver C
      PORTB |= (1 << PB1);  // SD high for Driver A
      ADMUX = 5;
      ACSR |= (1 << ACIS1) | (1 << ACIS0);
      break;
    case 5: // Driver C is high, Driver B is low.
      disable_driver_pins();
      TCCR2A |= (1 << COM2B1);
      PORTD |= (1 << PD4);  // IN high for Driver C
      PORTB |= (1 << PB3);  // SD high for Driver B
      ADMUX = 4;
      ACSR &= ~(1 << ACIS0);
      break;
  }
}


void setup() {
  // Use port manipulation to assign pins.
  // Set pins 4, 3 and 5 to output
  DDRD = (1 << PD5) | (1 << PD3) | (1 << PD4); // PD5,4,3 is D5,4,3
  // Set pins 4, 3, 5 low
  PORTD = 0;
  // Set pins 9, 10 and 11 to output
  DDRB = (1 << PB3) | (1 << PB2) | (1 << PB1); // PB3-1 is D11-9
  // Set pins 9, 10 and 11 low
  PORTB = 0;


  TCCR1A = (1 << WGM10);  // Fast PWM mode 14
  TCCR1B = (1 << CS10);  // No prescaler
  // Timer2 setup (8-bit PWM on pins 3 and 11)
  TCCR2A = (1 << WGM20);  // Fast PWM mode 3
  TCCR2B = (1 << CS20);  // No prescaler


  // Analog comparator setting
  ACSR = (1 << ACI);   // Disable analog comparator initially
  ADCSRA = (0 << ADEN);  // Disable ADC
  ADCSRB = (1 << ACME);  // Enable ADC multiplexer for comparator
  // Set initial state and PWM
  adjust_pwm(100);  // Set PWM to 50/255

  pinMode(THROTTLE, INPUT);
  attachInterrupt(digitalPinToInterrupt(THROTTLE), pwm_reading_interrupt, CHANGE);
  pinMode(LED_BUILTIN, OUTPUT);
}



// Analog comparator ISR
ISR (ANALOG_COMP_vect) {
  // BEMF debounce
  for(int i = 0; i < 10; i++) {
    if(bldc_state & 1){
      if(!(ACSR & 0x20)) i -= 1;
    }
    else {
      if((ACSR & 0x20))  i -= 1;
    }
  }
 
  bldc_next(bldc_state);
  bldc_state++;
  bldc_state %= 6;
}

void start() {
  digitalWrite(LED_BUILTIN, HIGH);
  bldc_state = 0;
  adjust_pwm(100);

  for (int i = 5000; i > 100; i -= 20) {
    delayMicroseconds(i);
    bldc_next(bldc_state);
    bldc_state++;
    bldc_state %= 6;
  }
  motorActive = true;
  ACSR |= (1 << ACIE);  
}

void stop() {
  motorActive = false;
  disable_driver_pins();
  ACSR &= ~(1 << ACIE);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (!motorActive && pendingDutyValue > MIN_PWM_DUTY) 
    start();
  
  if (motorActive && pendingDutyValue < DEACTIVATION_THRESHHOLD)
    stop();

  if (motorActive) 
    adjust_pwm(pendingDutyValue);
  
  }
