 /*CH CHETHAN REDDY
 * Embedded and Electronics
 * Function used - USART_TRANSMIT1,USART_TRANSMIT,USART_INIT,ADC_INIT,ADC_START,TIMER_INIT,TIMER_START,TIMER_STOP,ISR FOR TIMER AND ADC
 * Global Variables used - choice,ADC1,ADC2,ADC3,count,time_taken
 */

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#define BAUD 9600
#define BAUD_RATE ((F_CPU)/(BAUD*16)-1)

int choice=3;//Choice 0=Measure Voltage 1=Measure Current 2=Measure Resistance 3=Measure Capacitance
int ADC1=0;//This is used in the capacitor code it is nothing but 63% of ADC2 
int ADC2=0;//This is used in the capacitor code it is used to store the maximum ADC value attained
float ADC3=0;//This variable is used for volatge,current,resistance part of the code and this stores the converted value from ADC

int count;//This count no of complete cycles for TIMER1
double time_taken;//this variable stores the timetaken for the capacitor to reach 37% of charge using TIMER1

void USART_TRANSMIT1(char ch)//fn name:-USART_TRANSMIT1 This function simply take single character as a input and display them on the serial monitor
{
  while(!((UCSR0A)&(1<<UDRE0)));
  UDR0=ch;
}

void USART_TRANSMIT(float ch)//fn name:-USART TRANSMIT Takes float as a input
{
  int bef_dec,aft_dec;//Used for storing the integer part and the decimal part of the given float number
  char dec_bef[3],dec_aft[3];//Character buffer variables
  aft_dec=(ch-floor(ch))*100;//two decimal accuracy
  bef_dec=floor(ch);
  itoa(bef_dec,dec_bef,10);//Converts the integer to a string
  itoa(aft_dec,dec_aft,10);
  for(int i=0;i<sizeof(dec_bef);i++)
  USART_TRANSMIT1(dec_bef[i]);
  USART_TRANSMIT1('.');
  for(int i=0;i<sizeof(dec_aft);i++)//ch=12.37
  USART_TRANSMIT1(dec_aft[i]);//USART_TRANSMIT(ch);
  USART_TRANSMIT1(' ');//On serial monitor 12 .37 is displayed
}

void USART_INIT()//fn name:-USART_INT This function initializes the USART communication mode
{
  UBRR0=103;//This is for baud=9600;
  UCSR0B=(1<<TXEN0);
  UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);//USART_INT();
}

void ADC_INIT()//fn name:-ADC_INIT Initializing the ADC conversion
{
  ADMUX=(1<<REFS0)|(1<<MUX0)|(1<<MUX2);
  ADCSRA=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS0);//ADC_INIT();
  ADC_START();
}

void ADC_START()//fn name:-ADC_START Starts the ADC
{
  ADCSRA=(1<<ADSC)|(1<<ADEN);//ADC_START();
}

void TIMER_INIT()//fn name:-TIMER_INIT Initializing the timer
{
  TIMSK1=(1<<TOIE1);//TIMER_INIT();
}

void TIMER_START()//fn name:-TIMER_START Prescalar is set in this function so the timer starts once this function is run
{ TCCR1B=1<<CS11;}//TIMER_START();

void TIMER_STOP()//fn name:-TIMER_STOP Clears the prescalar values hence stopping the timer
{TCCR1B=0;}//TIMER_STOP();

ISR(TIMER1_OVF_vect)//Everytime the timer reaches its max the count is increased so by the end count gives the total number of cycles
{
  count++;
}

ISR(ADC_vect)//Set the Analog conversion flag to zero after every conversion
{
  ADCSRA&=!(1<<ADIF);
}

int main()
{
  USART_INIT();
  Serial.begin(9600);
  DDRC=0x00;
  sei();
  ADC_INIT();
  while(1)
  {
    while(!(ADCSRA & (1 << ADIF)));
    ADCSRA|=1<<8;
    ADC_START();

    if(choice==0)//The choice 0 is for voltage measurement
    {
      DDRD=0x00;
      ADC3=5*ADC;
      ADC3=ADC3/96;//VOLTAGE IN VOLTS
      //Serial.println(ADC3);
      USART_TRANSMIT(ADC3);//....
      _delay_ms(500);//Delay created because the values are being shown on the screen very fast
    }
    
    else if(choice==1)//The choice 1 is for current measurement
    {
      DDRD=0x00;
      ADC3=(5*ADC);
      ADC3=ADC3/(11*960);//Current in mA
      USART_TRANSMIT(ADC3);//....
      _delay_ms(500);
    }
    
    else if(choice==2)//The choice 2 is for resistance measurement 
    {
      DDRD=0x00;
      ADC3=(11*960)/ADC;
      ADC3=ADC3-10;
      USART_TRANSMIT(ADC3);//....
      _delay_ms(500);
    }
    
    else if(choice==3)//The choice 3 is for capacitor measurement
    {
       DDRC=0x00;
       DDRD=0x10;//Taken as the source for charging the capacitor
       sei();
       TIMER_INIT();//Timer intialized
       ADC_INIT();
  
       PORTD=(1<<PORTD4);
       ADC_START();
       do//This loop is to find the maximum value of charge that can be stored in the capacitor
      {
       ADC1=ADC;//First assigned to ADC1
       _delay_ms(3000);//Delay is created beacaue the increment is exponentially slow after some time so giving the ADC some time before start 
       ADC_START();
       while(!(ADCSRA & (1<< ADIF)));
      }while(ADC!=ADC1);//New ADC value is compared and then checked with the the previous ADC value if they are same then the loop is exited

       ADC_START();
       ADC2=ADC1;//ADC2 Stores the maximum charge of the capacitor
       ADC1=ADC2*0.37;//ADC store the 37% of charge for time constant calculation
       USART_TRANSMIT(ADC2);//....
       USART_TRANSMIT(ADC1);//....
       count=0;
       while(ADC>=ADC2)
       {ADC_START();}
  
        ADC_START();
        TIMER_START();
       while(ADC>ADC1)//Running the loop until the charge on the capacitor is greater than the 37% of charge once it reaches the loop is exited so the last time_taken value is capacitance
       {
        ADC_START();
        time_taken=(count*65025*0.0005)+(0.0005*TCNT1); //1 TICK TAKES 5e-7s
        time_taken=time_taken/100; //time_taken by the resistance for discharge in this case im using 100kohm resistor to discharge so we can capicatanc in mu-Farad 
        USART_TRANSMIT(time_taken);//....
        choice=5;//Setting this to prevent the loop from running again
       }
         TIMER_STOP();  //Stopping the timer
       }
      }
      return 0;
   }
