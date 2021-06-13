/*
 * PIC18F4550 LCD16x2 Header File
 * http://www.electronicwings.com
 */ 


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  

#include <pic18f4550.h>
#include "Configuration_Header_File.h"

#define _XTAL_FREQ 8000000
/*********************Definition of Ports********************************/

#define RS LATD0                    /*PIN 0 of PORTB is assigned for register select Pin of LCD*/
#define EN LATD1                    /*PIN 1 of PORTB is assigned for enable Pin of LCD */
#define ldata LATD                  /*PORTB(PB4-PB7) is assigned for LCD Data Output*/ 
#define LCD_Port TRISD

/*********************Proto-Type Declaration*****************************/

void MSdelay(unsigned int );        /*Generate delay in ms*/
void LCD_Init();                    /*Initialize LCD*/
void LCD_Command(unsigned char );   /*Send command to LCD*/
void LCD_Char(unsigned char x);     /*Send data to LCD*/
void LCD_String(const char *);      /*Display data string on LCD*/
void LCD_String_xy(char, char , const char *);
void LCD_Clear();                   /*Clear LCD Screen*/

#endif	/* XC_HEADER_TEMPLATE_H */

