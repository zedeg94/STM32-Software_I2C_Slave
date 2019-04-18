/*
 * sw_i2c.h
 *
 *  Created on: 13 апр. 2019 г.
 *      Author: Ilya
 */

#ifndef SW_I2C_H_
#define SW_I2C_H_

#include "stdint.h"

typedef struct I2C_State_Struct{
	uint8_t sdaState		:	1;
	uint8_t sclState		:	1;
	uint8_t startStopState	:	1;
	uint8_t rwState			:	1;
	uint8_t wfa				:	1;	//Wait for ACK
	uint8_t wfa_clr			:	1;
	uint8_t byte_in			:	1;
	uint8_t cur_slave		:	1;
	uint8_t	last_byte		:	1;
	//uint8_t					:	1;
}I2C_State_Struct;

typedef struct I2C_DataByte_Struct{
	uint8_t dataByte;
	uint8_t ackByte			:	1;
	uint8_t inCnt			:	4;
} I2C_DataByte_Struct;

typedef struct I2C_AddrByte_Struct{
	uint8_t readWrite		:	1;
	uint8_t slaveAddr		:	7;
} I2C_AddrByte_Struct;


#define		I2C_STATE_READ_WRITE		0x08
#define		I2C_STATE_START_STOP		0x04
#define		I2C_STATE_SCL				0x02
#define		I2C_STATE_SDA				0x01

#define		I2C_STATE_START				1U
#define		I2C_STATE_STOP				0U
#define		I2C_STATE_READ				1U
#define		I2C_STATE_WRITE				0U

#define		I2C_BIT_NAK					1U
#define		I2C_BIT_ACK					0U

extern I2C_State_Struct i2cState;
extern I2C_DataByte_Struct i2cRecByte;
extern I2C_DataByte_Struct i2cSendByte;

extern uint32_t startCnt;
extern uint32_t stopCnt;

extern uint32_t recArray[256];
extern uint8_t recArrayCnt;
extern uint32_t sendArray[256];
extern uint8_t sendArrayCnt;

extern I2C_AddrByte_Struct devSlaveAddr;

void I2C_SDA_SetState(void);
void I2C_SCL_SetState(void);

#endif /* SW_I2C_H_ */
