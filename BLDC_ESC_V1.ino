
const byte MAX_PWM_DUTY = 255;
const byte MIN_PWM_DUTY = 50;
byte bldc_state = 0;

void setup() {
  // Use port manipulation to assign pins.
  // https://docs.arduino.cc/retired/hacking/software/PortManipulation/
  // https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf

  // Set pins 2, 3 and 5 to ouput
  DDRD = 0;
  DDRD |= (1 << PD5) | (1 << PD3) | (1 << PD2); // PD5-2 is D5-2
  // Set pins 2, 3, 5 low
  PORTD = 0;
  // Set pins 9, 10 and 11 to ouput
  DDRB = 0;
  DDRB |= (1 << PB3) | (1 << PB2) | (1 << PB1); // PB3-1 is D11-9
  // Set pins 9, 10 and 11 low
  PORTB = 0;

  
  TCCR1A = 0; 
  TCCR1B = 0;
  ICR1 = 511;
  TCCR1A = (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

  
  TCCR2A = (1 << WGM20);
  TCCR2B = (1 << CS20);

  // Initial duties
  OCR1A = 256;    // Pin 10 - 50% (0-511 range)
  OCR2A = 128;    // Pin 11 - 50% (0-255 range)  
  OCR2B = 128;    // Pin 3  - 50% (0-255 range)

  //TCCR1A |= (1 << COM1A1);    // Pin 9 starts PWM
  //TCCR2A |= (1 << COM2A1) | (1 << COM2B1);  // Pins 11 & 3 start PWM

  // Analog comparator setting
  ACSR   = 0x10;           // Disable and clear (flag bit) analog comparator interrupt
}

void loop() {
  // put your main code here, to run repeatedly:

}
