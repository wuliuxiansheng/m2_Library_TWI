/* Name: main.c
 * Author: Chao Liu (chao.liu0307@gmail.com)
 * Date: August 12, 2014
 */

//M2 I2C Slave Code

#include "m_general.h"
#include "m_twi.h"

#define I2C_ADDR (0x0A)

 
// other:
#define I2C_UNDEFINED        0xF2  // no state information available
#define I2C_ERROR            0xF1  // an error occurred on the I2C bus
#define M_RED_TOGGLE         0xA0  // toggle red led and send back some data to the master
#define M_GREEN_TOGGLE       0xA1  // toggle green led and send back some data to master
#define M_RED_ON             0xA2  // turn on the red led
#define M_GREEN_ON           0xA3  // turn on the green led

void process_command(void);

int main (void)
{
    m_clockdivide(0);
    twi_init(I2C_ADDR);
    while(1)
    {
        
        if (receive_done)
        {
            process_command();
            receive_done = false;
            TWI_ACK();
        }
    }
}


void process_command(void)
{
    if (twi_data.data_length > 255)
    {
        twi_data.command = I2C_ERROR;
        twi_data.data_length = 0;
        return;
    }
    
    switch (twi_data.command)
    {
            
        case M_RED_TOGGLE:
            m_red(TOGGLE);
            twi_data.command=M_RED_TOGGLE;
            twi_data.data_length=5;
            twi_data.data[0]=1;
            twi_data.data[1]=2;
            twi_data.data[2]=3;
            twi_data.data[3]=10;
            twi_data.data[4]=5;
            break;
            
        case M_GREEN_TOGGLE:
            m_green(TOGGLE);
            twi_data.command=M_GREEN_TOGGLE;
            twi_data.data_length=4;
            twi_data.data[0]=5;
            twi_data.data[1]=6;
            twi_data.data[2]=7;
            twi_data.data[3]=8;
            break;
            
        case M_GREEN_ON:
            m_green(ON);
            break;
            
        case M_RED_ON:
            m_red(ON);
            break;
    }
}



