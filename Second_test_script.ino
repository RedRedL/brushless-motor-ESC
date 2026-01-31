/*
Author: RedRedL
Creation Date: 13/01/2026
Version: 5.1 (Corrected)
References:
  https://docs.arduino.cc/retired/hacking/software/PortManipulation/
  https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
  https://docs.arduino.cc/language-reference/en/functions/math/map/
  https://simple-circuit.com/arduino-brushless-dc-motor-controller/
  https://www.youtube.com/watch?v=F8l9Z1HGYcU
*/

const byte MAX_PWM_DUTY = 255;
const byte MIN_PWM_DUTY = 50;
volatile byte bldc_state = 0;

void adjust_pwm(int duty) {
  if (duty < MIN_PWM_DUTY)
    duty = MIN_PWM_DUTY;
  else if (duty > MAX_PWM_DUTY)
    duty = MAX_PWM_DUTY;

  OCR1A = map(duty, 0, 255, 0, 511);
  OCR2A = duty;
  OCR2B = duty;
}

void disable_driver_pins() {
  PORTB &= ~((1 << PB1) | (1 << PB2) | (1 << PB3));
  PORTD &= ~((1 << PD2) | (1 << PD3) | (1 << PD5));

  TCCR1A &= ~(1 << COM1A1);
  TCCR2A &= ~((1 << COM2A1) | (1 << COM2B1));
}

void bldc_next(byte bldc_state) {
  switch(bldc_state){
    /*
    Driver A: SD -> TCCR1A::COM1A1 -> PORTB::PB1; IN -> PORTB::PB2
    Driver B: SD -> TCCR2A::COM2A1 -> PORTB::PB3; IN -> PORTD::PD5
    Driver C: SD -> TCCR2A::COM2B1 -> PORTD::PD3; IN -> PORTD::PD2
    */
    case 0: // Driver A is high, Driver B is low.
      disable_driver_pins();
      // Enable PWM on Driver A, set IN high
      TCCR1A |= (1 << COM1A1);
      PORTB |= (1 << PB2);  // IN high for Driver A
      // Set Driver B low (no PWM, just low)
      PORTB |= (1 << PB3);  // SD high for Driver B (but PWM disabled)
      break;
      
    case 1: // Driver A is high, Driver C is low.
      disable_driver_pins();

      TCCR1A |= (1 << COM1A1);
      PORTB |= (1 << PB2);  // IN high for Driver A

      PORTD |= (1 << PD3);  // SD high for Driver C

      break;
      
    case 2: // Driver B is high, Driver C is low.
      disable_driver_pins();

      TCCR2A |= (1 << COM2A1);
      PORTD |= (1 << PD5);  // IN high for Driver B

      PORTD |= (1 << PD3);  // SD high for Driver C
      break;
      
    case 3: // Driver B is high, Driver A is low.
      disable_driver_pins();

      TCCR2A |= (1 << COM2A1);
      PORTD |= (1 << PD5);  // IN high for Driver B

      PORTB |= (1 << PB1);  // SD high for Driver A
      break;

    case 4: // Driver C is high, Driver A is low.
      disable_driver_pins();

      TCCR2A |= (1 << COM2B1);
      PORTD |= (1 << PD2);  // IN high for Driver C

      PORTB |= (1 << PB1);  // SD high for Driver A
      break;
      
    case 5: // Driver C is high, Driver B is low.
      disable_driver_pins();

      TCCR2A |= (1 << COM2B1);
      PORTD |= (1 << PD2);  // IN high for Driver C

      PORTB |= (1 << PB3);  // SD high for Driver B
      break;
  }
}


void setup() {
  // Use port manipulation to assign pins.
  // Set pins 2, 3 and 5 to output
  DDRD = (1 << PD5) | (1 << PD3) | (1 << PD2); // PD5-2 is D5-2
  // Set pins 2, 3, 5 low
  PORTD = 0;
  // Set pins 9, 10 and 11 to output
  DDRB = (1 << PB3) | (1 << PB2) | (1 << PB1); // PB3-1 is D11-9
  // Set pins 9, 10 and 11 low
  PORTB = 0;

  // Timer1 setup (10-bit PWM on pin 9)
  ICR1 = 511;  // Top value for 10-bit PWM
  TCCR1A = (1 << WGM11);  // Fast PWM mode 14
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);  // No prescaler

  // Timer2 setup (8-bit PWM on pins 3 and 11)
  TCCR2A = (1 << WGM20) | (1 << WGM21);  // Fast PWM mode 3
  TCCR2B = (1 << CS20);  // No prescaler

  // Set initial state and PWM
  adjust_pwm(100);  // Set PWM to 100/255
  bldc_state = 0;
}

void loop() {
  bldc_next(bldc_state);
  delay(25);
  disable_driver_pins();
  bldc_state++;
  bldc_state %= 6;
  delay(50);
}