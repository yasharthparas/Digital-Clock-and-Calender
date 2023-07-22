//Process of Data Writing and Reading

/*
Write: start | Dev address /w | ack check | initial address |
ack check | seconds | ack check | minutes | ack check | 
hours | ack check | day | ack check | .... | stop.

Read: start | Dev address /r | ack check | initial address | 
ack check | start | Dev address /r | ack check | seconds |
ack | .... | stop.
*/

#include<reg51.h>
#include<intrins.h> //for operations like push, pop, testbit, character rotate left-right, integer rotate left-right, etc.
sbit scl = P2^0;
sbit sda = P2^1;   //i2c pins
sbit rs = P2^2;
sbit rw = P2^3;
sbit en = P2^4;    //control lines
void start();      //initiate serial i2c communication
void delay();      //delay
void check();      //acknowledgement check from slave to master
void lcd_display();      //lcd display
void lcd_init();         //lcd initialization
void lcddisp(unsigned char *);   //lcd display
void lcdcmd(unsigned char);    //lcd command
void lcddat(unsigned char);   //allows 1 bit data every time
void write(unsigned char);     //sends data from sda bit 1 by 1
void delay3();                  
unsigned char read();          //read from rtc device
void enter();                   //go to next line in serial port
void ack();                    
void stop();
void display();                //used to display on serial terminal 
void delay2();
void serial_init();
void ser_msg(unsigned char *);
void conversion(unsigned char);
unsigned char sec, min, hr, day, date, mon, yr, ch;
bit c = 0;
void main()
{
	serial_init();
	//start();
	/*write(0xd0);   //slave address write mode
	write(0x00);   //initial location address
	write(0x00);   //seconds
	write(0x30);   //minutes
	write(0x20);   //hours
	write(0x07);   //week day
	write(0x09);   //date
	write(0x07);   //month
	write(0x23);   //year
	stop();*/
	while(1)
	{
		serial_init();
		start();
		write(0xd0);     //dev address in write mode
		write(0x00);     //seconds register address from RTC
		start();
		write(0xd1);     //dev address read mode
		read();                 //reads seconds
		sec = ch;
		sec = sec & 0x7f;   //fixes some particular value from seconds register
		stop();             
		start();            //{
		write(0xd0);
		write(0xd1);
		start();             
		write(0xd1);
		read();               //reads minutes
		min = ch;
		min = min & 0x7f;
		stop();              //}
		start();             //{
		write(0xd0);
		write(0x02);
		start();              
		write(0xd1);
		read();                  //reads hours
		hr = ch;
		hr = hr & 0x3f;
		stop();                //}
		start();               //{
		write(0xd0);
		write(0x03);
		start();               
		write(0xd1);
		read();                  //reads day
		day = ch; 
		day = day & 0x0f;       //logical bitwise AND operation
		stop();                 //}
		start();                //{
		write(0xd0); 
		write(0x04);
		start();                  
		write(0xd1);
		read();                    //reads date
		date = ch;
		date = date & 0x03f;
		stop();                    //}
		start();                   //{
		write(0xd0);
		write(0x05);
		start();
		write(0xd1);
		read();                     //reads month
		mon = ch;
		mon = mon & 0x0f;
		stop();                    //}
		start();                    //{
		write(0xd0);
		write(0x06);
		start();
		write(0xd1);
		read();                     //reads year
		yr = ch;
		yr = yr & 0x3f;
		stop();                        //}
		display();
		lcd_display();
	}
}
void display()
{
	conversion(hr);
	conversion(min);
	conversion(sec);
	enter();
//conversion(day);
//enter();
	conversion(date);
	conversion(mon);
	conversion(yr);
	enter();
}
void conversion(unsigned char res)
{
	unsigned char v4,v5;
	v4 = res & 0x0f;      //LSB
	v5 = res & 0xf0;      //MSB
	v5 = v5 >> 4;         //Result in LSB position
	v4 = v4 | 0x30;
	v5 = v5 | 0x30;        //ASCII conversion
	SBUF = v5; while(TI == 0); TI = 0;     //values passed to UART terminal
	SBUF = v4; while(TI == 0); TI = 0;     //clears TI bit after every transmission
	SBUF = ':'; while(TI == 0); TI = 0;
}
void lcd_display()
{
	unsigned char v9, v10;
	lcd_init();
	lcddisp("TIME:");
	lcdcmd(0xc0);            //forces cursor to second line
	lcddisp("DATE:");
	v9 = hr & 0x0f;
	v10 = (hr & 0xf0) >> 4;
	lcdcmd(0x86);
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	lcddat(':');
	v9 = min & 0x0f;
	v10 = (min & 0xf0) >> 4;
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	lcddat(':');
	v9 = sec & 0x0f;
	v10 = (sec & 0xf0) >> 4;
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	lcdcmd(0xc6);
	v9 = date & 0x0f;
	v10 = (date & 0xf0) >> 4;
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	lcddat('/');
	v9 = mon & 0x0f;
	v10 = (mon & 0xf0) >> 4;
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	lcddat('/');
	v9 = yr & 0x0f;
	v10 = (yr & 0xf0) >> 4;
	lcddat(v10 | 0x30);
	lcddat(v9 | 0x30);
	delay3();
}
void delay3()
{
	unsigned int v11;
	for(v11 = 0; v11 < 32000; v11++);
//for(v11 = 0; v11 < 60000; v11++);
}	
void lcd_init()
{
	lcdcmd(0x38);     //activating all the crystals
	lcdcmd(0x01);      //clear the screen
	lcdcmd(0x10);      //shift the cursor right
	lcdcmd(0x0c);      //display on cursor blinking
	lcdcmd(0x80);      //fixes the cursor with first line first position 
}
void enter()       //enter the cursor postion to the next line 
{
	SBUF = 0X0D;
	while(TI == 0);
	TI = 0;
}
void write(unsigned char val)
{
	unsigned char v2 = 0x80, v3, v5;
	v5 = val;
	for(v3 = 0; v3 < 8; v3++)
	{
		sda = v5 & v2;
		scl = 1;
		delay();
		scl = 0;
		v5 = v5 << 1;
	}
	c = sda;      //acknowledgement verification
	scl = 1;
	delay();
	scl = 0;
	if(c == 1)
	{
		stop();
		while(1);
	}
}
void stop()
{
	scl = 1;
	sda = 0;
	delay();
	sda = 1;
	scl = 0;
	delay();
}
void delay()
{
	_nop_();
	_nop_();
}
unsigned char read()
{
	unsigned char v16;
	bit m = 0;
	for(v16 = 0; v16 <= 7; v16++)
	{
		scl = 1;
		delay();
		m = sda;
		scl = 0;
		if(m == 1)
		{
			ch |= 0x01;
		}
		if(v16 <= 6)
			ch = ch << 1;
		delay();
	}
	return ch;
}
void start()
{
	scl = 1;
	sda = 1;
	delay();
	sda = 0;
	delay();
	scl = 0;
}
void serial_init()
{
	SCON = 0X50;
	TMOD = 0X20;    //Timer 1 Mode 1
	TH1 = -3;      //baud rate for uC
	TR1 = 1;        //Timer initialization
}
void lcddisp(unsigned char *m)
{
	unsigned char p;
	for(p = 0; p[m] != '\0'; p++)
	lcddat(m[p]);
}
void lcddat(unsigned char val)
{
	P1 = val;
	rs = 1;
	rw = 0;
	en = 1;
	delay2();
	en = 0;
}
void lcdcmd(unsigned char val)
{
	P1 = val;
	rs = 0;
	rw = 0;
	en = 1;
	delay2();
	en = 0;
}
void delay2()
{
	unsigned int v1;
	for(v1 = 0; v1 < 2000; v1++);
}