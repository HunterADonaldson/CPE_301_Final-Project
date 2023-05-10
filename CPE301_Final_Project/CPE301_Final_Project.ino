
//CPE 301: Final Project
//Author: Hunter Donaldson
//Due: 5/9/23

// Libraries
#include <dht.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <Stepper.h>
#include <Wire.h>

// UART Pointers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0; //Usart control register A
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1; //Usart control register B
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2; //Usart control register C
volatile unsigned int  *myUBRR0  = (unsigned int *)0x00C4; //Usart Baud rate register
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6; //Usart Data register
// ADC pointers
volatile unsigned char* myADMUX   = 0x7C; //ADC Multiplexer Selection Register
volatile unsigned char* myADCSRB  = 0x7B; //ADC Control and Status Register B
volatile unsigned char* myADCSRA  = 0x7A; //ADC Control and Status Register A
volatile unsigned int* ADC_DATA = 0x78; //ADC Data Register
// GPIO Pointers
// Port B (pins 13,12,11,10,50,51,52,53)
volatile unsigned char* portB = 0x25; //Port B data register
volatile unsigned char* DDR_B = 0x24; //Port B data direction Register
volatile unsigned char* Pin_B = 0x23; //Port B input pins
//Port H (pins 9,8,7,6,16,17)
volatile unsigned char* portH = 0x102; //Port C data register
volatile unsigned char* DDR_H = 0x101; //Port C data direction register
volatile unsigned char* Pin_H = 0x100; //Port C input pins
//Port G (pins 4,39,40,41)
volatile unsigned char* portG = 0x34; //Port G data register
volatile unsigned char* DDR_G = 0x33; //Port G data direction register
volatile unsigned char* Pin_G = 0x32; //Port G input pins
//Port E (pins 5,3,2,1,0)
volatile unsigned char* portE = 0x2E; //Port E data register
volatile unsigned char* DDR_E = 0x2D; //Port E data direction register
volatile unsigned char* Pin_E = 0x2C; //Port E input pins
//Port A (pins 22,23,24,25,26,27,28,29,)
volatile unsigned char* portA = 0x22; //Port A data register
volatile unsigned char* DDR_A = 0x21; //Port A data direction register
volatile unsigned char* Pin_A = 0x20; //Port A input pins
// Timer Pointers
volatile unsigned char* myTCCR1A = 0x80; // Timer/Counter control register A
volatile unsigned char* myTCCR1B = 0x81; // Timer/Counter control register B
volatile unsigned char* myTCCR1C = 0x82; // Timer/COunter control register C
volatile unsigned char* myTIMSK1 = 0x6F; // Timer/Counter 1 Interrupt mask register
volatile unsigned char* myTIFR1  = 0x36; // Timer/Counter 1 Interrupt flag register
volatile unsigned int*  myTCNT1  = 0x84; // Timer/Counter 1 (high or low?)

// START OF LIBRARY SET UPS
#define RDA 0x80
#define TBE 0x20

// Stepper
#define IN1 23
#define IN2 25
#define IN3 27
#define IN4 29
const int stepsPerRev = 2038;
Stepper myStepper = Stepper(stepsPerRev, IN1, IN2, IN3, IN4);

// Temp / Hemidity
#define DHT11_PIN 6
dht DHT;
int Temp_Threshold = 24;

// LCD
#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// RTC
RTC_DS1307 rtc;

// Water level sensor
int WL_Threshold = 120;
int Water_level = 0;

// State values
volatile char state; //Not sure if this will work

void setup() {
  // setup the UART
  U0Init(9600);
  // setup the ADC
  adc_init();
  // Water level
  *DDR_H |= 0x10; // PH4, Water level Sensor
  // LCD
  lcd.begin(16,2);
  lcd.print("hello, world");
  delay(500);
  lcd.clear();
  // DC Motor
  *DDR_G |= 0x04; // PG2, DC fan speed
  *DDR_G |= 0x02; // PG1, fan direction 1
  *DDR_G |= 0x01; // PG0, fan direction 2
  // LEDS
  *DDR_B |= 0x01; // PB0, Green LED
  *DDR_B |= 0x02; // PB1, Yellow LED
  *DDR_B |= 0x04; // PB2, Red LED
  *DDR_B |= 0x08; // PB3, Blue LED
  // Buttons
  *DDR_B |= 0x10; // PB4, Button 3
  *DDR_H |= 0x40; // PH6, Button 2
  *DDR_H |= 0x20; // PH5, Button 1
  // RTC
  rtc.begin();
  rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop() 
{
/* ALL TESTS ARE WORKING TEST CODE USED TO ENSURE CIRCUIT IS WORKING */
  //test RTC
  DateTime now = rtc.now();

  //test if water sensor, IT WORKS
  *portH |= 0x10;
  Water_level = adc_read(0);
  U0putChar('w');
  U0putChar(':');
  print(Water_level);
  if(Water_level < WL_Threshold)
  {
    lcd.print("ERROR");
  }

  //test DHT11 sensor and LCD, THEY WORK
  int chk = DHT.read11(DHT11_PIN);
  lcd.print("Temp =");
  lcd.print(DHT.temperature);
  lcd.setCursor(0,1);  
  lcd.print("Humidity =");
  lcd.print(DHT.humidity);

  
  //test Stepper motor, IT WORKS
  myStepper.setSpeed(10);
  myStepper.step(stepsPerRev);
  delay(500);

  //test DC motor, Not sure how to get this working
  *portG |= 0x02;
  *portG &= 0x01;  
  lcd.clear();


/*
EVERYTHING BELLOW IS COMMENT IN THE LOOP IS MY FULL ATTEMPTED AT CODING
THE LOOP TO OPERATE THE IDEL, DISABLED, ERROR, AND RUNNING STATES

  state = 'D';
  *portB |= 0x10; // Button 3, RESET
  *portH |= 0x40; // Button 2, SYSTEM OFF
  *portH |= 0x20; // Button 1, SYSTEM ON
  *portH |= 0x10; // Turn on Water Level Sensor Pin
  Water_level = adc_read(0);

  switch (state)
  {
    case 'D':
      disabled();
      if(*portH = 0x10)
      {
        state = 'I'; // turns system on
      }
      break;
    case 'I':
      break;
    case 'E':
      break;
    case 'R':
      break;
  }
*/
}

//ADC Functions
void adc_init()
{
  //setup Register A
  *myADCSRA |= 0b10000000; // set bit 
  *myADCSRA &= 0b11011111; // clear bit 6 to disable the ADC trigger mode
  *myADCSRA &= 0b11110111; // clear bit 3 to disable the ADC interrut
  *myADCSRA &= 0b11111000; // clear bit 0-2 to set prescaler selection to slow reading
  //setup Register B
  *myADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *myADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  //setup MUX
  *myADMUX &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *myADMUX |= 0b01000000; // set bit 6 to 1 for AVCC analog reference
  *myADMUX &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *myADMUX &= 0b11100000; // clear bit 4-0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *myADMUX &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *myADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the cahnnel selection bits, but renove the most signicificant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *myADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *myADMUX += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *myADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*myADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *ADC_DATA;
}

//UART Functions
void U0Init(int U0baud)
{
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud -1);
  *myUCSR0A = 0x20; // USART Data register empty
  *myUCSR0B = 0x18; // USART data register empty Interrupt Enable, and Receive Enable n
  *myUCSR0C = 0x06; // Character size 8-bit
  *myUBRR0  = tbaud; // set baud rate
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
void U0putChar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE) == 0);
  *myUDR0 = U0pdata;
}
// States
void idel()
{
// monitors temp and water level, Green LED is on
// Temp and Humidity should be displayed
// Fan off
// Goes to Disabled state if stop button is pressed
  *portB |= 0x01; // Turn on LED
}
void error()
{
// ERROR state if water level is too low, Red LED is on
// goes to Idle state if reset button is pressed
// goes to Disabled state if stop button is pressed
  *portB |= 0x04; // Turn on LED
}
void running()
{
// Running state if Temp > Threshold, Blue LED is on
// Temp and Humidity should be displayed
// On entry: Fan on, On Exit: Fan off
// goes to Disabled state if stop button is pressed
  *portB |= 0x08; // Turn on LED
}
void disabled()
{
// Disabled state if stopped at any point, Yellow LED is on
// goes to Idel if start button is pressed
  *portB |= 0x02; // Turn on LED
}
// other needed functions
void print(unsigned int Water_level)
{
  if(Water_level >= 1000)
  {
    U0putChar(Water_level / 1000 + '0');
    Water_level = Water_level % 1000;
  }
  if(Water_level >= 100)
  {
    U0putChar(Water_level / 100 + '0');
    Water_level = Water_level % 100;
  }
  if(Water_level >= 10)
  {
    U0putChar(Water_level / 10 + '0');
    Water_level = Water_level % 10;
  }
  U0putChar(Water_level + '0');
  U0putChar('\n');  
}
