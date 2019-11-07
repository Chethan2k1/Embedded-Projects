/*
 * Author Name: CH Chethan Reddy
 * Domain : Embedded and Electronics
 * Sub-domain: Embedded Systems
 * Functions: req, assign, PWM_INIT, PWM_START, PWM_STOP
 * Global variables : i,j,k,SINE_TABLE[]
 */
#include<math.h>
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

int i=0,j=3;//i is used as index for the SINE_TABLE[] and Varying j will give diffrent pitches 
char k;//k stores the notes A,B,C,D,E,F,G

uint8_t SINE_TABLE[]= //These are the values used for producing the sine wave and which will be used to assign a value to the OCR1B
{
  0,9,37,78,127,176,217,245,255,245,217,176,127,78,37,9
};

int req(float frq)//This function is to calculate the OCR1A taking frquency required as input
{
  int z;
  z=(125000/frq);//16e6/(16*8)=125000 16e6 is the clock frequency, 16 is the no of elements in SINE_TABLE() and 8 is for prescalar
  return z;
}

float assign()//This is to assign the frequency by using the variables j,k 
{
  float freq;
  switch(k)
  {
    case 'A':
     freq=55.00*pow(2,(j-1)); //55.00 is the frequency of A1; to get frequency of any pitch we simply multiply with 2^(j-1) where j is the pitch 
     break;
    case 'B':
     freq=61.74*pow(2,(j-1));
     break;
    case 'C':
     freq=32.70*pow(2,(j-1));
     break;
    case 'D':
     freq=36.71*pow(2,(j-1));
     break;
    case 'E':
     freq=41.20*pow(2,(j-1));
     break;
    case 'F':
     freq=43.65*pow(2,(j-1));
     break;
    case 'G':
     freq=49.00*pow(2,(j-1));
     break;
   }
   return freq;
}

void PWM_INIT(int x)//This is for initializing the PWM of TIMER1 with OCR1A as a argument 
{
  TCCR1A=(1<<COM1B1)|(1<<WGM11)|(1<<WGM10);//Non-Inverting modee
  OCR1B=0;
  OCR1A=x;
  TIMSK1=(1<<TOV1);
}

void PWM_START()//This is for starting the PWM 
{
  TCCR1B=(1<<WGM12)|(1<<WGM13)|(1<<CS11);//Setting the prescalar to 8 and setting top=OCR1A
}

void PWM_STOP()//Stopping the PWM
{
  TCCR1B=0;
}

ISR(TIMER1_OVF_vect) //Timer1 Overflow interrupt
{
  if(i<(sizeof(SINE_TABLE)-1))
  {
    i++;
  }
  else
  {
    i=0;
  }
  OCR1B=(SINE_TABLE[i]*OCR1A)/255;//To make the output of the PWM of constant voltage regardless of the value of OCR1A 
}

int main()
{
  int x;
  Serial.begin(9600);
  DDRB=1<<PORTB2;//D10 is set to output for the PWM Output(OCR1B pin)
  DDRD=0x00;//Setting all of them to input for the buttons
  PORTB=(1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2)|(1<<PORTB3)|(1<<PORTB4)|(1<<PORTB5);//This is enabling pullups for the PWM and buttons
  PORTD=(1<<PORTD7)|(1<<PORTD6);//Enabling pullups for buttons
  sei();//Enabling the global interrupt register
  x=req(assign()); 
  PWM_INIT(x);
  PWM_START();
  while(1)
  {
    if(~PINB&(1<<PORTB0))//Reading the pin to check whether the particular pin is High or Low
    {
      k='C';//If it is low then assigning k='C' 
      PWM_INIT(req(assign()));//Initializing the value of OCR1A to PWM and also initializing the PWM
      PWM_START();//Starting the Timer1
      while(~PINB&(1<<PORTB0));//Waiting till the button is pressed
    }
    else if(~PINB&(1<<PORTB1))
    {
      k='D';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PINB&(1<<PORTB1));
    }
    else if(~PIND&(1<<PORTD6))
    {
      k='E';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PIND&(1<<PORTD6));
    }
    else if(~PINB&(1<<PORTB3))
    {
      k='F';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PINB&(1<<PORTB3));
    }
    else if(~PINB&(1<<PORTB4))
    {
      k='G';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PINB&(1<<PORTB4));
    }
    else if(~PINB&(1<<PORTB5))
    {
      k='A';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PINB&(1<<PORTB5)); 
    }
    else if(~PIND&(1<<PORTD7))
    {
      k='B';
      PWM_INIT(req(assign()));
      PWM_START();
      while(~PIND&(1<<PORTD7)); 
    }
    PWM_STOP();//To ensure that no sound comes from the speaker when the button is no longer pressed
  }
}
