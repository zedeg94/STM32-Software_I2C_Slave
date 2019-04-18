/*
 * sw_i2c.c
 *
 *  Created on: 13 ���. 2019 �.
 *      Author: Ilya
 */

#include "sw_i2c.h"
#include "main.h"

I2C_State_Struct i2cState;
I2C_DataByte_Struct i2cByteDR;

uint32_t startCnt = 0;
uint32_t stopCnt = 0;

uint32_t recArray[256] = {0};
uint8_t recArrayCnt = 0;

uint32_t sendArray[256]	=	{0};
uint8_t sendArrayCnt = 0;

I2C_AddrByte_Struct devSlaveAddr = { 0, 0x38};
I2C_AddrByte_Struct tmpByte = {0};

void I2C_OnRise(void);
void I2C_ProcessTransmitTail_Rise(void);
void I2C_ProcessBitIn(void);
void I2C_RecByte(void);
void I2C_ProcByteIn(void);


void I2C_OnFall(void);
void I2C_ProcessBitOut(void);
void I2C_SendBitAck(void);
void I2C_ProcessTransmitTail_Fall(void);

void I2C_ProcRead(void);
void I2C_ProcByteOut(void);

void I2C_SDA_SetState(void)
{
	if (i2cState.sclState == 1)
	{
		if (i2cState.sdaState == 1)
		{
			i2cState.startStopState = I2C_STATE_STOP;
			stopCnt++;
			i2cState.rwState = I2C_STATE_WRITE;
			i2cState.byte_in = 0;
			i2cState.cur_slave = 0;

			i2cByteDR.inCnt = 0;
		}
		else
		{
			i2cState.startStopState = I2C_STATE_START;
			startCnt++;
		}
	}
}

void I2C_SCL_SetState(void)
{
	if (i2cState.startStopState == I2C_STATE_START)
	{
		if (i2cState.sclState == 1)	//SCL � ������� ������
		{
			I2C_OnRise(); //���������� 1-8 ��� ��� ��������� � 9-� ��� (ACK) ��� ��������
		}
		else	//SCL � ������ ������
		{
			I2C_OnFall();	//�������� 1-8 ��� ��� �������� � 9-� ���  (ACK) ��� ���������
		}
	}
}


void I2C_OnRise(void)
{
	if (i2cByteDR.inCnt > 0x07 )	//��������� ����� ��������
	{
		GPIOD->BSRR = SW_I2C_SCL_Pin << 16;
		GPIOA->BSRR = GPIO_PIN_1 <<16;
		I2C_ProcessTransmitTail_Rise();

		i2cState.sclState = 0;
		I2C_OnFall();
		GPIOD->BSRR = SW_I2C_SCL_Pin ;
		GPIOA->BSRR = GPIO_PIN_1;
	}
	else	//��������� �������� ���� �� �������
	{
		I2C_ProcessBitIn();
	}
}


void I2C_OnFall(void)
{
	if(i2cState.rwState == I2C_STATE_READ)	//��������
	{
		if (i2cByteDR.inCnt < 0x08)
		{
			I2C_ProcessBitOut();	//������ SDA �� ���������� ������
		}
		else
		{
				I2C_ProcessTransmitTail_Fall();	//�� 8-� ��������� SDA
		}
	}
	else	//����
	{
		if (i2cByteDR.inCnt > 0x07)
		{
			if (i2cByteDR.inCnt == 0x08)
			I2C_SendBitAck();
			else
			{
				I2C_ProcessTransmitTail_Fall();

				i2cByteDR.inCnt = 0;
			}
		}
	}
}

void I2C_ProcessTransmitTail_Rise(void)
{
	i2cByteDR.inCnt++;

	if(i2cState.rwState == I2C_STATE_READ)//������� ������ ACK/NACK �� ������� (��� ������)
	{
		i2cByteDR.ackByte = i2cState.sdaState;
		if (i2cByteDR.ackByte == I2C_BIT_NAK)
		{
			i2cState.startStopState = I2C_STATE_STOP;
		}
		else
		{
		//���������� ����. �����
			I2C_ProcByteOut();	//����� ��� else
		}
	}
	else
	{
		i2cState.rwState = tmpByte.readWrite;
		I2C_ProcRead();
	}
}

void I2C_ProcRead(void)	//TODO:: �������� � ���������
{
	if (i2cState.rwState == I2C_STATE_READ)
	{
		I2C_ProcByteOut();
	}
}

void I2C_ProcessBitIn(void)
{
	i2cByteDR.inCnt++;	//����������� ������� �������� ��� �� 1

	if(i2cState.rwState == I2C_STATE_WRITE)
	{
		i2cByteDR.dataByte = (i2cByteDR.dataByte << 1) | i2cState.sdaState; //��������� ���

		if (i2cByteDR.inCnt == 0x08) //���� ������ 8 ���
		{
			I2C_RecByte(); //������������ ���������� ����
		}
	}
}


void I2C_RecByte(void)
{
	if(i2cState.cur_slave)
	{
		I2C_ProcByteIn();
	}
	else
	{
		tmpByte = *(I2C_AddrByte_Struct*)&i2cByteDR.dataByte;

		if (devSlaveAddr.slaveAddr == tmpByte.slaveAddr)
		{
			i2cState.cur_slave = 1;
			i2cByteDR.ackByte = I2C_BIT_ACK;
		}
		else
			i2cState.startStopState = I2C_STATE_STOP;
	}
}


void I2C_ProcByteIn(void)
{
	sendArrayCnt = i2cByteDR.dataByte;
}

void I2C_ProcessBitOut(void)
{
	uint32_t tmp_dataByte = i2cByteDR.dataByte;
			tmp_dataByte >>= i2cByteDR.inCnt;
	GPIOD->ODR = (GPIOD->ODR & ~SW_I2C_SDA_Pin) | ((tmp_dataByte << 9) & SW_I2C_SDA_Pin);
	/*--------------------------*/
	/*--------------------------*/
}

void I2C_ProcByteOut(void)
{
	i2cByteDR.dataByte = sendArray[sendArrayCnt++];
	i2cByteDR.inCnt = 0;
}

void I2C_SendBitAck(void)
{
	GPIOD->ODR = (GPIOD->ODR & ~SW_I2C_SDA_Pin) | (i2cByteDR.ackByte << 9);
	/*--------------------------*/
	/*--------------------------*/
}

void I2C_ProcessTransmitTail_Fall(void)
{
		GPIOD->BSRR = SW_I2C_SDA_Pin ;
		/*--------------------------*/
		/*--------------------------*/
}
