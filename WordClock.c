/*
 * WordClock.c
 *
 *  Author: Brendan Vercoelen
 *  http://no8hacks.com
 * 
 * A functional clock which displays the time in words in increments of 5 minutes. It
 * uses multiplexing to control a gridded array of LEDs and uses 74HC595 Shift
 * Registers. It is written for the Attiny2313 but could be easily be adapted for any
 * Microcontroller. The RTC Clock is the DS1307.
 *
 * A full write up on the build is available at:
 * http://no8hacks.com
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>

// I2C Implementation by doctek
// http://www.instructables.com/id/I2C_Bus_for_ATtiny_and_ATmega/
#include "USI_TWI_Master.h"

// Output Pins
#define Clear_Enable                PORTB |= 0b00000010
#define Set_Enable                  PORTB &= ~0b00000010
#define Shift_Clk_H                 PORTD |= 0b1000000
#define Shift_Clk_L                 PORTD &= ~0b1000000
#define Latch_Clk_H                 PORTD |= 0b0010000
#define Latch_Clk_L                 PORTD &= ~0b0010000
#define Data_H                      PORTD |= 0b0100000
#define Data_L                      PORTD &= ~0b0100000
#define Reset                       PORTB &= ~0b00000001
#define Reset_Clear                 PORTB |= 0b00000001

//Input Pins
#define UP                          ~PIND & 0b0000100
#define DOWN                        ~PIND & 0b0001000

int main(void);                     // Main function
void setclock(int, int);            // Function sets the time on the clock face
unsigned char getTime(void);        // Function gets the time from the RTC
void setTime(int, int);             // Function sets the time in the RTC
unsigned char int2bcd(int);         // Function converts integer to binary coded decimal
int bcd2int(unsigned char);         // Function converts binary coded decimal to integer
void increment(void);               // Increment time to next 5 minutes
void decrement(void);               // Decrement time to previous 5 minutes

// Output 
int output[9] = {0,0,0,0,0,0,0,0,0};
unsigned char hour, minute;
    
unsigned char TWI_targetSlaveAddress = 0b1101000;
unsigned char messageBuf[5];

int main(void)
{
    // Set output pins
    DDRB = 0b11111111;
    DDRD = 0b1110011;
    PORTB = 0b00000000;
    PORTD = 0b0000000;
  
    
    // Prepare Shift Register
    Set_Enable;
    Latch_Clk_L;
    Reset;
    Reset_Clear;
    
    // Setup I2C
    USI_TWI_Master_Initialise();
    
    getTime();
    
    int count = 0;
    unsigned char transition = 0;
    
    while(1)
    {
        
        // If UP Button is pressed
        if (UP && count > 30)
        {
            increment();
            count = 0;
        }
        
        // If DOWN button is pressed
        if (DOWN && count > 30)
        {
            decrement();
            count = 0;
        }
        
        // If time has not been updated in ~1 second
        if (count > 100)
        {
            transition = getTime();
            count = 0;
        }
        
        if (transition > 0 && count%7 == 0)
        {
            for (int i=8; i > 0; i--)
            {
                output[i] = output[i-1];
            }
            output[0] = 0b01010111010*count >> 1;
        }
        
        count++;
        
        for (int i=0; i < 9; i++)
        {
            // If row has no data to display
            if (output[i] == 0)
            {
                continue; // Go to next row
            }
            
            // Shift in row location
            for (int j=0; j < 9; j++)
            {
                if (i == j)
                {
                    Data_H;
                }
                else
                {
                    Data_L;
                }
                
                // Shift Clock Pulse
                Shift_Clk_H;
                Shift_Clk_L;
            }
            
            // Shift in enabled columns
            for (int j=0; j < 11; j++)
            {
                if ((output[i] & (1 << j)) >> j)
                {
                    Data_H;
                }
                else
                {
                    Data_L;
                }
                
                // Shift Clock Pulse
                Shift_Clk_H;
                Shift_Clk_L;
            }    
            
            // Latch Clock Pulse
            Latch_Clk_H;
            Latch_Clk_L;
            
            _delay_ms(2);
        }
    }
}

void setClock(int h, int m)
{
    for (int i=0; i<9; i++)
    {
        output[i] = 0;
    }

    output[0] = 0b00000001111; // IT IS
    
    if (m > 4 && m < 35)
    {
        output[3] = 0b00000001111; // PAST
    }
    else if (m > 34)
    {
        output[3] = 0b00000011000; // TO
        h = (h + 1)%24;
    }
    
    switch (h)
    {
        case 0:
            output[6] = 0b00011111111; // MIDNIGHT
            break;
        case 1:
        case 13:
            output[7] = 0b01110000000; // ONE
            break;
        case 2:
        case 14:
            output[6] = 0b01110000000; // TWO
            break;
        case 3:
        case 15:
            output[4] = 0b11111000000; // THREE
            break;
        case 4:
        case 16:
            output[5] = 0b00000001111; // FOUR
            break;
        case 5:
        case 17:
            if (hour == 17 && minute < 5)
            {
                output[2] = 0b11110000000; // BEER
            }
            else
            {
                output[5] = 0b00011110000; // FIVE
            }
            break;          
        case 6:
        case 18:
            output[5] = 0b11100000000; // SIX
            break;
        case 7:
        case 19:
            output[8] = 0b00000011111; // SEVEN
            break;
        case 8:
        case 20:
            output[3] |= 0b11111000000; // EIGHT
            break;
        case 9:
        case 21:
            output[4] = 0b00000111100; // NINE
            break;
        case 10:
        case 22:
            output[4] = 0b00000000111; // TEN
            break;
        case 11:
        case 23:
            output[7] = 0b00000111111; // ELEVEN
            break;
        case 12:
            output[7] = 0b00111100000; // NOON
            break;
    }
    
    if (m < 5 && (h != 12 && h != 0))
    {
         output[8] |= 0b11111100000; // OCLOCK
    }
    else if ((m > 4 && m < 10) || m > 54)
    {
        output[2] = 0b00000001111; // FIVE
    }
    else if ((m> 9 && m < 15) || (m > 49 && m < 55))
    {
        output[2] = 0b00001110000; // TEN
    }
    else if ((m > 14 && m < 20) || (m > 44 && m < 50))
    {
        output[1] = 0b00001111111; // QUARTER
    }
    else if ((m > 19 && m < 25) || (m > 39 && m < 45))
    {
        output[0] = 0b11111101111; // ITIS TWENTY
    }
    else if ((m > 24 && m < 30) || (m > 34 && m < 40))
    {
        output[0] = 0b11111101111; // ITIS TWENTY
        output[2] = 0b00000001111; // FIVE
    }
    else if (m > 29 && m < 35)
    {
        output[1] = 0b11110000000; // HALF
    }
}

unsigned char getTime(void)
{
    int h;
    messageBuf[0] = (TWI_targetSlaveAddress<<1) | (FALSE<<TWI_READ_BIT);
    messageBuf[1] = 0x00;
    USI_TWI_Start_Read_Write(messageBuf, 2);
    
    messageBuf[0] = (TWI_targetSlaveAddress<<1) | (TRUE<<TWI_READ_BIT);
    USI_TWI_Start_Read_Write(messageBuf, 4);
    minute = bcd2int(messageBuf[2]);
    h = bcd2int(messageBuf[3]);
    
    if (hour != h) {
        hour = h;
        return 1; // Show transition effect on hour change
    }
    setClock(hour,minute);
    return 0;
}

void setTime(int h, int m)
{
    hour = h;
    minute = m;
    
    messageBuf[0] = (TWI_targetSlaveAddress<<1) | (FALSE<<TWI_READ_BIT);
    messageBuf[1] = 0x00;
    messageBuf[2] = 0;
    messageBuf[3] = int2bcd(m);
    messageBuf[4] = int2bcd(h);
    USI_TWI_Start_Read_Write(messageBuf, 5);
    
    setClock(h,m);
}

unsigned char int2bcd(int val)
{
    return ((val / 10) << 4) | (val % 10);
}

int bcd2int(unsigned char val)
{
    return ((val & 0b11110000) >> 4)*10 + (val & 0b00001111);
}

void increment(void)
{
    if (minute > 54) {
        setTime((hour+1)%24,00);
    }
    else
    {
        setTime(hour,(minute/5+1)*5);
    }
}

void decrement(void)
{
    if (minute < 5) {
        setTime((hour+23)%24,55);
    }
    else
    {
        setTime(hour,(minute/5-1)*5);
    }
}