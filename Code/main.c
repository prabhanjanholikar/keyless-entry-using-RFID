/* Keyless Entry using RFID with PIC18F4550 */
#include <p18f4550.h>
#include "Configuration_header_file.h"
#include "16x2_LCD_4bit_File.h"
#include <stdbool.h>
#include <string.h>

#define write_port LATD /* latch register to write data on port */
#define read_port PORTD /* PORT register to read data of port */
#define Direction_Port TRISD
#include <xc.h> // include processor files - each processor file is guarded.
#define _XTAL_FREQ 8000000
#define MAX 20
int serial_data_recv = 0;
int button_click=0;
/********Definition of Ports***********/
#define RS LATB2   //PIN 0 of PORTD is assigned for register select Pin of LCD
#define EN LATB3   //PIN 1 of PORTD is assigned for enable Pin of LCD
#define ldata LATB //PORTD(PD4-PD7) is assigned for LCD Data Output
#define LCD_Port TRISB
#define ATX IO1
#define TX TRISCbits.TRISC6
#define RX TRISCbits.TRISC7
#define SW TRISCbits.TRISC0

void MSdelay(unsigned int);      //Generate delay in ms
void LCD_Init();                 //Initialize LCD
void LCD_Command(unsigned char); //Send command to LCD
void LCD_Char(unsigned char x);  //Send data to LCD
void LCD_String(const char *);   //Display data string on LCD
void LCD_String_xy(char, char, const char *);
void LCD_Clear();        //Clear LCD Screen
unsigned char keyfind(); // function to find pressed key
void LCD_String_xypos (char,char ,const char *);

int button = 1;
unsigned char col_loc, rowloc, temp_col;

unsigned char keypad[4][4] = {'7', '8', '9', '/',
                              '4', '5', '6', '*',
                              '1', '2', '3', '-',
                              ' ', '0', '=', '+'};

void LCD_Init()
{
    LCD_Port = 0;      //PORT as Output Port
    MSdelay(15);       //15ms,16x2 LCD Power on delay
    LCD_Command(0x02); /*send for initialization of LCD 
                                     for nibble (4-bit) mode */
    LCD_Command(0x28); /*use 2 line and 
                                     initialize 5*8 matrix in (4-bit mode)*/
    LCD_Command(0x01); //clear display screen
    LCD_Command(0x0c); //display on cursor off
    LCD_Command(0x06); //increment cursor (shift cursor to right)
}

void LCD_Command(unsigned char cmd)
{
    ldata = (ldata & 0x0f) | (0xF0 & cmd); //Send higher nibble of command first to PORT
    RS = 0;                                //Command Register is selected i.e.RS=0
    EN = 1;                                //High-to-low pulse on Enable pin to latch data
    NOP();
    EN = 0;
    MSdelay(1);
    ldata = (ldata & 0x0f) | (cmd << 4); /*Send lower nibble of command to PORT */
    EN = 1;
    NOP();
    EN = 0;
    MSdelay(3);
}

void LCD_Char(unsigned char dat)
{
    ldata = (ldata & 0x0f) | (0xF0 & dat); //Send higher nibble of data first to PORT
    RS = 1;                                //Data Register is selected
    EN = 1;                                //High-to-low pulse on Enable pin to latch data
    NOP();
    EN = 0;
    MSdelay(1);
    ldata = (ldata & 0x0f) | (dat << 4); //Send lower nibble of data to PORT
    EN = 1;                              //High-to-low pulse on Enable pin to latch data
    NOP();
    EN = 0;
    MSdelay(3);
}

void LCD_String(const char *msg)
{
    while ((*msg) != 0)
    {
        LCD_Char(*msg);
        msg++;
    }
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char location = 0;
    if (row <= 1)
    {
        location = (0x80) | ((pos)&0x0f); //Print message on 1st row and desired location
        LCD_Command(location);
    }
    else
    {
        location = (0xC0) | ((pos)&0x0f); //Print message on 2nd row and desired location
        LCD_Command(location);
    }

    LCD_String(msg);
}

void LCD_String_xypos(char row,char pos,const char *msg)
{  
	LCD_String(msg);
}

void LCD_Clear()
{
    LCD_Command(0x01); //clear display screen
}

void MSdelay(unsigned int val)
{
    unsigned int i, j;
    for (i = 0; i < val; i++)
        for (j = 0; j < 165; j++)
            ; /*This count Provide delay of 1 ms */
}

unsigned char keyfind()
{

    Direction_Port = 0xf0; //PORTD.0-PORTD.3 as a Output Port and PORTD.4-PORTD.7 as a Input Port
    unsigned char temp1;

    write_port = 0xf0; //Make lower nibble as low(Gnd) and Higher nibble as High(Vcc)
    do
    {
        do
        {

            col_loc = read_port & 0xf0; //mask port with f0 and copy it to col_loc variable

        } while (col_loc != 0xf0 && !serial_data_recv);  //Check initially at the start there is any key pressed
        col_loc = read_port & 0xf0; //mask port with f0 and copy it to col_loc variable
    } while (col_loc != 0xf0 && !serial_data_recv);

    MSdelay(50);
    write_port = 0xf0; //Make lower nibble as low(Gnd) and Higher nibble as High(Vcc)
    do
    {
        do
        {
            col_loc = read_port & 0xf0;
        } while (col_loc == 0xf0 && !serial_data_recv); //Wait for key press
        col_loc = read_port & 0xf0;
    } while (col_loc == 0xf0 && !serial_data_recv); //Wait for key press

    MSdelay(20);

    col_loc = read_port & 0xf0;

    while (1 && !serial_data_recv)
    {
        write_port = 0xfe;          //make Row0(D0) Gnd and keep other row(D1-D3) high
        col_loc = read_port & 0xf0; //Read Status of PORT for finding Row
        temp_col = col_loc;
        if (col_loc != 0xf0)
        {
            rowloc = 0;              //If condition satisfied get Row no. of key pressed
            while (temp_col != 0xf0) //Monitor the status of Port and Wait for key to release
            {
                temp_col = read_port & 0xf0; //Read Status of PORT for checking key release or not
            }
            break;
        }

        write_port = 0xfd;          //make Row1(D1) Gnd and keep other row(D0-D2-D3) high
        col_loc = read_port & 0xf0; //Read Status of PORT for finding Row
        temp_col = col_loc;
        if (col_loc != 0xf0)
        {
            rowloc = 1;              //If condition satisfied get Row no. of key pressed
            while (temp_col != 0xf0) //Monitor the status of Port and Wait for key to release
            {
                temp_col = read_port & 0xf0; //Read Status of PORT for checking key release or not
            }
            break;
        }

        write_port = 0xfb;          //make Row0(D2) Gnd and keep other row(D0-D1-D3) high
        col_loc = read_port & 0xf0; //Read Status of PORT for finding Row
        temp_col = col_loc;
        if (col_loc != 0xf0)
        {
            rowloc = 2;              //If condition satisfied get Row no. of key pressed
            while (temp_col != 0xf0) //Wait for key to release
            {
                temp_col = read_port & 0xf0; //Read Status of PORT for checking key release or not
            }
            break;
        }

        write_port = 0xf7;          //make Row0(D3) Gnd and keep other row(D0-D2) high
        col_loc = read_port & 0xf0; //Read Status of PORT for finding Row
        temp_col = col_loc;
        if (col_loc != 0xf0)
        {
            rowloc = 3;              //If condition satisfied get Row no. of key pressed
            while (temp_col != 0xf0) //Wait for key to release
            {
                temp_col = read_port & 0xf0; //Read Status of PORT for checking key release or not
            }
            break;
        }
    }

    while (1 && !serial_data_recv)
    {

        if (col_loc == 0xe0)
        {
            return keypad[rowloc][0]; //Return key pressed value to calling function
        }
        else if (col_loc == 0xd0)
        {
            return keypad[rowloc][1]; //Return key pressed value to calling function
        }
        else if (col_loc == 0xb0)
        {
            return keypad[rowloc][2]; //Return key pressed value to calling function
        }
        else

        {
            return keypad[rowloc][3]; //Return key pressed value to calling function
        }
    }

    MSdelay(300);
}
void interrupt ISR()
{   
    LCD_Clear();
	LCD_String_xy(0, 0, "Data Received");
	LCD_Command(0xC0);
	LCD_String_xy(0XC0,0XC0, "  WELCOME  ");
	//MSdelay(1000);
	if (button_click==0)
	{
		//button_click = button_click+1;
		LCD_Command(0x94);
		LCD_String_xypos(0X94,0, "12345: ABCDE");
		//MSdelay(1000);
		button_click = button_click+1;
	}
	else if (button_click>=1)
	{
		LCD_Command(0x94);
		LCD_String_xypos(0X94,0, "12345: ABCDE");
		LCD_Command(0xD4);
		LCD_String_xypos(0xD4,3, "98765: PQRST");
		//MSdelay(1000);
		button_click = button_click+1;
	}
	
	serial_data_recv = 1;
    INTCONbits.INT0IF=0;
}
void main(void)
{
    int period;    /* Set period in between two steps */
    OSCCON = 0x72; /* Use Internal Oscillator 8MHz */
    TRISA = 0x00;  /* Make PORTA as output */
    period = 100;
    int val = 0;
    TX = 0;
    RX = 1;
    SW = 1;

	//SPBRG = 0x00139H;
	//SPBRGH = 0x02;      // 0x00139H for 9600 baud 
	//TXSTA = 0x24;       // TX enable BRGH=1
	//RCSTA = 0x90;       // continuous RX
	//BAUDCON = 0x08;     // BRG16 = 1
     
	char key;
    bool test;
    char Password[MAX];
    char preset[] = "12345";
    int Count = 0;
    LCD_Init(); /* initialize LCD16x2 in 4-bit mode */
    LCD_Clear();
	LCD_String_xy(0, 0, "Welcome to ");
	MSdelay(1000);
	LCD_Command(0xC0);
	LCD_String_xy(0XC0,0XC0, "KeylessEntrySystem");
	MSdelay(3000);
	LCD_Clear();
	LCD_String_xy(0, 0, "Press a Button or ");
	
    LCD_Command(0xC0); /* display pressed key on 2nd line of LCD */
    LCD_String_xy(0XC0,0XC0, "Enter Password ");
	
	TRISBbits.TRISB0=1;
    INTCON2=0x00;		/* Set Interrupt on falling Edge*/
    INTCONbits.INT0IF=0;	/* Clear INT0IF flag*/
    INTCONbits.INT0IE=1;	/* Enable INT0 external interrupt*/
    INTCONbits.GIE=1;		/* Enable Global Interrupt*/
    if (strcmp(RX,"12345")==0)
    {
        //LCD_Command(0x94);
		//LCD_String_xypos(0X94,0, "WELCOME");
		goto X;
    }

    else
    {
        while (1)
        {
            key = keyfind(); /* find a pressed key */
			LCD_Char(key);   /* display pressed key on LCD16x2 */
            Password[Count] = key;
            Count = Count + 1;
            button &= PORTCbits.RC0;
            if (strcmp("12345", Password) == 0 || serial_data_recv)
            {
                goto X;
                break;
            }
        }

        //test = strcmp(preset, Password);
        if (serial_data_recv)
        {
        X:
            while (1)
            {
                /* Rotate Stepper Motor clockwise with Half step sequence */
                for (int i = 0; i < 1; i++)
                {
                    LATA = 0x09;
                    MSdelay(period);
                    LATA = 0x08;
                    MSdelay(period);
                    LATA = 0x0C;
                    MSdelay(period);
                    LATA = 0x04;
                    MSdelay(period);
                    LATA = 0x06;
                    MSdelay(period);
                    LATA = 0x02;
                    MSdelay(period);
                    LATA = 0x03;
                    MSdelay(period);
                    LATA = 0x01;
                    MSdelay(period);
                }
                LATA = 0x09; /* Last step to initial position */
                MSdelay(period);
                MSdelay(3000);

                /* Rotate Stepper Motor Anticlockwise with Full step sequence */
                for (int i = 0; i < 1; i++)
                {
                    LATA = 0x09;
                    MSdelay(period);
                    LATA = 0x03;
                    MSdelay(period);
                    LATA = 0x06;
                    MSdelay(period);
                    LATA = 0x0C;
                    MSdelay(period);
                }
                LATA = 0x09;
                MSdelay(period);
                MSdelay(3000);
            }
        }
    }
}
