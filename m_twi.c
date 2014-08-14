// -----------------------------------------------------------------------------
// M2 TWI IIC subsystem
// version: 1.0
// date: August 12, 2014
// authors: Chao Liu (chao.liu0307@gmail.com)
// -----------------------------------------------------------------------------

#include "m_twi.h"

// TWI Slave Status:
#define TX_ADDR_ACK          0xA8  // receive slave address + read bit and ACK'd
#define TX_ADDR_ACK_ARB_LOST 0xB0  // receive slave address + read bit and ACK'd, and master arbitration is lost
#define TX_BYTE_ACK          0xB8  // send the byte in TWDR and get an ACK
#define TX_BYTE_NACK         0xC0  // send the byte in TWDR and get a NACK
#define TX_FINAL_BYTE_ACK    0xC8  // send the last byte in TWDR and get an ACK

#define RX_ADDR_ACK          0x60  // receive slave address + write bit and ACK'd
#define RX_ADDR_ACK_ARB_LOST 0x68  // receive slave address + write bit and ACK'd, and master arbitration is lost
#define RX_GEN_ACK           0x70  // receive general call address and ACK'd
#define RX_GEN_ACK_ARB_LOST  0x78  // receive general call address and ACK'd, and master arbitration is lost
#define RX_ADDR_DATA_ACK     0x80  // receive data and ACK'd
#define RX_ADDR_DATA_NACK    0x88  // receive data and NACK'd
#define RX_GEN_DATA_ACK      0x90  // receive data on general call address and ACK'd
#define RX_GEN_DATA_NACK     0x98  // receive data on general call address and NACK'd
#define RX_STOP_RESTART      0xA0  // receive a STOP or repeated START condition


volatile bool receive_done;
struct TWI_data twi_data;

void twi_init(uint8_t slave_address)
{
// set the frequency in master mode
//    clear (TWSR, TWPS0);
//    clear (TWSR, TWPS1);
//    TWBR = 12;
    TWAR = (slave_address << 1) & 0xfe;
    TWI_ACK();
    sei();
    receive_done = false;
}

ISR (TWI_vect)
{
    static uint8_t *transmission_ptr = (uint8_t *)&twi_data;
    static bool receiving = false;
    const uint8_t status = TWSR & 0xF8;
    
    switch (status)
    {
        case TX_ADDR_ACK:
        case TX_ADDR_ACK_ARB_LOST:
            transmission_ptr = (uint8_t *)&twi_data;
            TWDR = *transmission_ptr++;
            TWI_ACK();
            break;
            
        case TX_BYTE_ACK:
            // continue to send data
            TWDR = *transmission_ptr++;
            if ( (transmission_ptr >= ((uint8_t *)&twi_data) + 2 &&
                  transmission_ptr >= ((uint8_t *)&twi_data) + 2 + twi_data.data_length) ||
                transmission_ptr >= ((uint8_t *)&twi_data) + 2 + TWI_BUFFER_SIZE)
            {
                transmission_ptr = (uint8_t *)&twi_data;  // reset buffer to default address
            }
            TWI_ACK();
            break;
            
        case TX_BYTE_NACK:
            transmission_ptr = (uint8_t *)&twi_data;  // reset buffer to default address
            TWI_ACK();
            break;
            
        case RX_ADDR_ACK:
            
        case RX_ADDR_ACK_ARB_LOST:
            // begin to receive an order
            receiving = true;
            twi_data.data_length = 0;
            transmission_ptr = (uint8_t *)&twi_data;
            TWI_ACK();
            break;
            
        case RX_ADDR_DATA_ACK:
            *transmission_ptr++ = TWDR;
            
            if ( (transmission_ptr >= ((uint8_t*)&twi_data) + 2 &&
                  transmission_ptr >= ((uint8_t*)&twi_data) + 2 + twi_data.data_length) ||
                transmission_ptr >= ((uint8_t*)&twi_data) + 2 + TWI_BUFFER_SIZE)
            {
                TWI_NACK();
                receiving = false;
                receive_done = true;
                transmission_ptr = (uint8_t *)&twi_data;  // reset buffer to default address
            }
            else
            {
                TWI_ACK();  // continue receiving data
            }
            break;
            
        case RX_STOP_RESTART:
            
        case TX_FINAL_BYTE_ACK:
            
        case RX_ADDR_DATA_NACK:
            
        case RX_GEN_ACK:
            
        case RX_GEN_ACK_ARB_LOST:
            
        case RX_GEN_DATA_ACK:
            
        case RX_GEN_DATA_NACK:
            
        default:
            if (receive_done)
            {
                TWI_NACK();
            }
            else if (receiving)
            {
                receiving = false;
                receive_done = true;
                
                transmission_ptr = (uint8_t *)&twi_data;  // reset buffer to default address
                TWI_NACK();
            }
            else
            {
                transmission_ptr = (uint8_t *)&twi_data;
                receiving = false;
                TWI_ACK();
            }
            break;
    }
}


