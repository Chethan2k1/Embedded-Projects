/*
 * Author Name:CH CHETHAN REDDY
 * Domain:Embedded and Analog Electronics
 * Sub-Domain:Embedded Systems
 * Functions:-EXT_INIT(),3-ISR's,timestamp(),USART_INIT,USART_TRANSMIT1(),USART_TRANSMIT,USART_RECIEVE,BCD,i2c_start,i2c_init,i2c_end,RTC_INIT,RTC_READ,RTC_WRITE
 * Variables:- second,minute,hour,dat,month,year,alarm,timer,time_stamp,m,s,ms,start_flag,choice,alarm_flag
 */
#include<avr/io.h>
#include<string.h>

unsigned char second=0x30,minute=0x13,hour=0x18,date=0x06,month=0x07,year=0x19;//These variables store the data of the RTC in hex form
unsigned char alarm[5],timer[5],time_stamp[9];//alarm stores the alarm time,timer store the timer time,timer_stamp stores the current time in a string for easy comparison
int m=0,s=0,ms=0;//These are used for stopwatch which can measure with milliseconds accuracy
volatile int start_flag=0,choice=0,alarm_flag=0; //startflag{0,1} Used for start-stop,choice{0-RTC,1-Alarm,2-Stopwatch,3-Timer),alarm_flag-to check if alarm time reached

void EXT_INIT()//External Interrupt initiate
{
  EICRA=(1<<ISC01)|(1<<ISC11);  //Falling edge
  EIMSK=(1<<INT0)|(1<<INT1); //Enabling INT0 and INT1
}

ISR(INT0_vect) //ISR for changing the value of the start_flag
{
  if(start_flag==1)
  start_flag=0;
  else
  start_flag=1;
  EICRA=0;
  EIFR&=!(1<<INTF0);
  EXT_INIT();
}

ISR(INT1_vect) //ISR for changing the value of the choice
{
  if(choice==3)
  choice=0;
  else
  choice++;
  EICRA=0;
  EIFR&=!(1<<INTF1);
  EXT_INIT();
}

void timestamp(int m,int s,int ms) //On giving m,s,ms it converts it to string form and stores it in char time_stamp[]
{
  int i=8,j;
  for(j=3;j>0;j--)
  {
    time_stamp[i]='0'+ms%10;
    ms=ms/10;
    --i;
  }
  time_stamp[i]=':';
  --i;
  for(j=2;j>0;j--)
  {
    time_stamp[i]='0'+s%10;
    s=s/10;
    --i;
  }
  time_stamp[i]=':';
  --i;
  for(j=2;j>0;j--)
  {
    time_stamp[i]='0'+m%10;
    m=m/10;
    --i;
  }
}

void USART_INIT() //This function initializes the USART communication mode
{
  UBRR0=103;//This is for baud=9600;
  UCSR0B=(1<<TXEN0)|(1<<RXEN0);
  UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);//USART_INT();
}

void USART_TRANSMIT(char ch) // This function simply take single character as a input and display them on the serial monitor
{
  while(!((UCSR0A)&(1<<UDRE0)));
  UDR0=ch;
}

void USART_TRANSMIT1(unsigned char ch[]) //This function transmits a string of characters
{
  int i;
  for(i=0;i<strlen(ch);i++)
  {
    USART_TRANSMIT(ch[i]);
  }
}

char USART_RECIEVE1(unsigned char ch[],int len) //This Recieves a string of characters
{
  int i=0;
  while(i<len)
  {
    while(!(UCSR0A&(1<<RXC0)));
    ch[i]=UDR0;
    i++;
  }
}

void BCD(unsigned char data) //This converts the Real time sent by the DS1307(RTC) which is in BCD and saves in charachters and sends through USART_TRANSMIT
{
  time_stamp[0]='0'+(hour>>4);
  USART_TRANSMIT('0'+(hour>>4));
  time_stamp[1]='0'+(hour & 0X0F);
  USART_TRANSMIT('0'+(hour & 0X0F));
  time_stamp[2]=':';              //Also stores the real time in a string for comparision
  USART_TRANSMIT(':');
  time_stamp[3]='0'+(minute>>4);
  USART_TRANSMIT('0'+(minute>>4));
  time_stamp[4]='0'+(minute & 0X0F);
  USART_TRANSMIT('0'+(minute & 0X0F));
  USART_TRANSMIT(':');
  USART_TRANSMIT('0'+(second>>4));
  USART_TRANSMIT('0'+(second & 0X0F));
  USART_TRANSMIT(' ');
  USART_TRANSMIT('0'+(date>>4));
  USART_TRANSMIT('0'+(date & 0X0F));
  USART_TRANSMIT('-');
  USART_TRANSMIT('0'+(month>>4));
  USART_TRANSMIT('0'+(month & 0X0F));
  USART_TRANSMIT('-');
  USART_TRANSMIT('0'+(year>>4)); 
  USART_TRANSMIT('0'+(year & 0X0F));
}

void i2c_init(void) //Initiates the ITC
{
  TWSR=0x00;
  TWBR=0x47;
  TWCR=(1<<TWEN);
}

void i2c_start(void)  //Sends the Start bit
{
  TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while(!(TWCR&(1<<TWINT)));
}

void i2c_end(void) //Sends the stop bit
{
  TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
  while(!(TWCR&(1<<TWSTO))); //...
}

void i2c_write(unsigned char data) //Sends data to the slave(DS1307)
{
  TWDR=data;
  TWCR=(1<<TWINT)|(1<<TWEN);
  while(!(TWCR&(1<<TWINT)));
}

uint8_t i2c_read(unsigned char ack_val)  //Recieves data from the slave and the ack_val{0,1} 1-ACK 0-NACK
{
  TWCR=(1<<TWINT)|(1<<TWEN)|(ack_val<<TWEA);
  while(!(TWCR&(1<<TWINT)));
  return TWDR;
}

void RTC_init(void) //Initiating the RTC i.e 1)Initiating the RTC 2)Addressing the slave 3)Selecting the control bit register 4)Writing on to the control bit
{
  i2c_init();
  i2c_start();
  i2c_write(0xD0);
  i2c_write(0x07);
  i2c_write(0x00);
  i2c_end();
}

void RTC_set_time_date(unsigned char second,unsigned char minute,unsigned char hour,unsigned char date,unsigned char month,unsigned char year) 
{
  i2c_start();   
  i2c_write(0xD0);
  i2c_write(0x00);  //Addressing the first data register i.e 0X00 after that on sending it auto increments
  i2c_write(second);
  i2c_write(minute);
  i2c_write(hour);
  i2c_end();
  i2c_start();  //Ending and Starting again to prevent writing day
  i2c_write(0xD0);
  i2c_write(0x04);
  i2c_write(date);
  i2c_write(month);
  i2c_write(year);
  i2c_end();  
}

void RTC_read()  //For reading the registers for Real time
{ 
  i2c_start();   
  i2c_write(0xD0); //Addressing the slave
  i2c_write(0x00); //Setting the pointer to 0x00
  i2c_end();
  
  i2c_start();
  i2c_write(0xD1);
  second=i2c_read(1);
  minute=i2c_read(1);
  hour=i2c_read(1);
  i2c_read(1);  //Just simply reading to increment the pointer
  date=i2c_read(1);
  month=i2c_read(1);
  year=i2c_read(0);
  i2c_end();
}

void RTC_display() //This function converts the BCD form of read data and sends them to Serial Monitor
{
  BCD(time_stamp);
  USART_TRANSMIT(' ');
  USART_TRANSMIT('\n');
}

void timer_start()  //Starts the timer
{
  TCCR0A=(1<<WGM01);
  OCR0A=250; //Takes 1ms for compare match
  TIMSK0=(1<<OCIE0A);  //Interrupt on compare match
  TCCR0B=(1<<CS01)|(1<<CS00); //Prescalar 64
}

ISR(TIMER0_COMPA_vect) //ISR for comparison this basically increments the time every time by milli seconds
{
  ms++;
  if(ms>=1000)
  {
    s+=ms/1000;
    ms=ms%1000;
    if(s>=60)
    {
      m+=s/60;
      s=s%60;
    }
  }
  timer_start();
}
 
void timer_stop() //Stops the timer as prescalar is set
{
  TCCR0B=0;
  TIMSK0=0;
}

int main()
{
  USART_INIT();
  EXT_INIT();
  sei();
  DDRD=0x00;
  DDRB=0X00;
  PORTB=1<<PORTB0;
  PORTD=(1<<PORTD2)|(1<<PORTD3);
  RTC_init();
  RTC_set_time_date(second,minute,hour,date,month,year);
  
while(1)
{
  RTC_read();
  switch(choice)
  {
    case 0:  //This is for RTC and this also checks for the alarm continously simultaneously showing the time
        m=0;
        s=0;
        ms=0;
        if(strncmp(alarm,time_stamp,5)==0&&alarm_flag==1) //Checks for whether the hour and minute is same as in alarm and time stamp(Real time)
        { 
          USART_TRANSMIT1("Wake up\t"); //This is where the buzzer thing should come
          DDRB=1<<PORTB0;  //If true Buzzer turns on
          alarm_flag=1;
          RTC_display();
          _delay_ms(1000);
        }
        else
        {
          DDRB=0x00;
           RTC_display();
          _delay_ms(1000);
        }
        if(start_flag==1) //On clicking on the first button it increments start_flag and shows the menu
        {
          alarm_flag=0;
          choice=0;
          DDRB=0x00; //Clears all the flags to prevent previous data effects and the buzzer(Pressing this also means to stop the buzzer in case if its ON)
          USART_TRANSMIT1("Choose your choice 0.RTC 1.Alarm 2.Stopwatch 3.Timer\n");
          while(start_flag==1);//waits till clicking of button again(In the video i mentioned this as "ENTER")
        }
        break;
    case 1: //Alarm mode
      USART_TRANSMIT1("Alarm time :");
      UCSR0B=0;
      USART_INIT(); //This clears the recieving buffer(was getting junk value previously)
      USART_RECIEVE1(alarm,5);//Recieving the alarm data from the user
      USART_TRANSMIT1(alarm);
      alarm_flag=1;
      USART_TRANSMIT1("\tAlarm set\n");
      choice=0;
      break;
   case 2: //Stopwatch mode
      USART_TRANSMIT1("\nPRESS to START");
      while(start_flag==0);
      timer_start();  //Once the button is pressed the stopwatch starts
      while(start_flag==1) //Until the START/STOP button is pressed the timer increments ms,s,m in ISR
      {
        timestamp(m,s,ms);
        USART_TRANSMIT1(time_stamp);
        USART_TRANSMIT('\n');
      }
      timer_stop();
      USART_TRANSMIT1(time_stamp);
      _delay_ms(2000);
      USART_TRANSMIT('\n');
      choice=0; //Comes back to RTC
      break;
   case 3: //Timer mode
      USART_TRANSMIT1("Time:"); 
      USART_RECIEVE1(timer,5);  //Recieving the timer time
      USART_TRANSMIT1(timer);
      USART_TRANSMIT1("\nPRESS to START");
      while(start_flag==0); //Waits till the start button is pressed
      timer_start();
      while(strncmp(timer,time_stamp,5)) //checking whether the timer is same as time_stamp(comparing only the seconds and the minutes part)
      {
        timestamp(m,s,ms);
      }
      USART_TRANSMIT1("\nPRESS to STOP");
      while(start_flag==1)
      DDRB=1<<PORTB0; //Waiting till the button is pressed till that time the buzzer keeps on buzzing
      DDRB=0x00; 
      timer_stop();
      choice=0;  //Returns to the RTC
      break;
  }
}
  return 0;
}
