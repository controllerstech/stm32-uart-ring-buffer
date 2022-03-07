/*
 * UartRingbuffer.c
 *
 *  Created on: 10-Jul-2019
 *      Author: Controllerstech
 *
 *  Modified on: 11-April-2020
 */

#include "UartRingbuffer.h"
#include <string.h>

/**** define the UART you are using  ****/

UART_HandleTypeDef huart2;

#define uart &huart2

#define TIMEOUT_DEF 500  // 500ms timeout
uint16_t timeout;

/* put the following in the ISR 

extern void Uart_isr (UART_HandleTypeDef *huart);
extern uint16_t timeout;

*/

/****************=======================>>>>>>>>>>> NO CHANGES AFTER THIS =======================>>>>>>>>>>>**********************/


ring_buffer rx_buffer = { { 0 }, 0, 0};
ring_buffer tx_buffer = { { 0 }, 0, 0};

ring_buffer *_rx_buffer;
ring_buffer *_tx_buffer;

void store_char(unsigned char c, ring_buffer *buffer);


void Ringbuf_init(void)
{
  _rx_buffer = &rx_buffer;
  _tx_buffer = &tx_buffer;

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(uart, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(uart, UART_IT_RXNE);
}

void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % UART_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if(i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

/* checks, if the entered string is present in the giver buffer ?
 */
static int check_for (char *str, char *buffertolookinto)
{
	int stringlength = strlen (str);
	int bufferlength = strlen (buffertolookinto);
	int so_far = 0;
	int indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx])
		{
			indx++;
			if (indx>stringlength) return 0;
		}
	if (str[so_far] == buffertolookinto[indx])
	{
		while (str[so_far] == buffertolookinto[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == stringlength);
	else
	{
		so_far =0;
		if (indx >= bufferlength) return -1;
		goto repeat;
	}

	if (so_far == stringlength) return 1;
	else return -1;
}

int Uart_read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if(_rx_buffer->head == _rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
    return c;
  }
}

/* writes a single character to the uart and increments head
 */
void Uart_write(int c)
{
	if (c>=0)
	{
		int i = (_tx_buffer->head + 1) % UART_BUFFER_SIZE;
		while (i == _tx_buffer->tail);

		_tx_buffer->buffer[_tx_buffer->head] = (uint8_t)c;
		_tx_buffer->head = i;

		__HAL_UART_ENABLE_IT(uart, UART_IT_TXE); // Enable UART transmission interrupt
	}
}

/* checks if the new data is available in the incoming buffer
 */
int IsDataAvailable(void)
{
  return (uint16_t)(UART_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % UART_BUFFER_SIZE;
}

/* sends the string to the uart
 */
void Uart_sendstring (const char *s)
{
	while(*s) Uart_write(*s++);
}

void GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto)
{
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int so_far = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[so_far] != buffertocopyfrom[indx]) indx++;
	if (startString[so_far] == buffertocopyfrom[indx])
	{
		while (startString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == startStringLength) startposition = indx;
	else
	{
		so_far =0;
		goto repeat1;
	}

	so_far = 0;

repeat2:
	while (endString[so_far] != buffertocopyfrom[indx]) indx++;
	if (endString[so_far] == buffertocopyfrom[indx])
	{
		while (endString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == endStringLength) endposition = indx-endStringLength;
	else
	{
		so_far =0;
		goto repeat2;
	}

	so_far = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		buffertocopyinto[indx] = buffertocopyfrom[i];
		indx++;
	}
}

void Uart_flush (void)
{
	memset(_rx_buffer->buffer,'\0', UART_BUFFER_SIZE);
	_rx_buffer->head = 0;
	_rx_buffer->tail = 0;
}

int Uart_peek()
{
  if(_rx_buffer->head == _rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

/* copies the data from the incoming buffer into our buffer
 * Must be used if you are sure that the data is being received
 * it will copy irrespective of, if the end string is there or not
 * if the end string gets copied, it returns 1 or else 0
 * Use it either after (IsDataAvailable) or after (Wait_for) functions
 */
int Copy_upto (char *string, char *buffertocopyinto)
{
	int so_far =0;
	int len = strlen (string);
	int indx = 0;

again:
	while (Uart_peek() != string[so_far])
		{
			buffertocopyinto[indx] = _rx_buffer->buffer[_rx_buffer->tail];
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
			indx++;
			while (!IsDataAvailable());

		}
	while (Uart_peek() == string [so_far])
	{
		so_far++;
		buffertocopyinto[indx++] = Uart_read();
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

/* must be used after wait_for function
 * get the entered number of characters after the entered string
 */
int Get_after (char *string, uint8_t numberofchars, char *buffertosave)
{
	for (int indx=0; indx<numberofchars; indx++)
	{
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);  // wait until some data is available
		if (timeout == 0) return 0;  // if data isn't available within time, then return 0
		buffertosave[indx] = Uart_read();  // save the data into the buffer... increments the tail
	}
	return 1;
}

/* Waits for a particular string to arrive in the incoming buffer... It also increments the tail
 * returns 1, if the string is detected
 */
// added timeout feature so the function won't block the processing of the other functions
int Wait_for (char *string)
{
	int so_far =0;
	int len = strlen (string);

again:
	timeout = TIMEOUT_DEF;
	while ((!IsDataAvailable())&&timeout);  // let's wait for the data to show up
	if (timeout == 0) return 0;
	while (Uart_peek() != string[so_far])  // peek in the rx_buffer to see if we get the string
	{
		if (_rx_buffer->tail != _rx_buffer->head)
		{
			_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		}

		else
		{
			return 0;
		}
	}
	while (Uart_peek() == string [so_far]) // if we got the first letter of the string
	{
		// now we will peek for the other letters too
		so_far++;
		_rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}




void Uart_isr (UART_HandleTypeDef *huart)
{
	  uint32_t isrflags   = READ_REG(huart->Instance->SR);
	  uint32_t cr1its     = READ_REG(huart->Instance->CR1);

    /* if DR is not empty and the Rx Int is enabled */
    if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
    	 /******************
    	    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	    	      *          sequence: a read operation to USART_SR register followed by a read
    	    	      *          operation to USART_DR register.
    	    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	    	      *          USART_SR register followed by a write operation to USART_DR register.
    	    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	 *********************/
		huart->Instance->SR;                       /* Read status register */
        unsigned char c = huart->Instance->DR;     /* Read data register */
        store_char (c, _rx_buffer);  // store data in buffer
        return;
    }

    /*If interrupt is caused due to Transmit Data Register Empty */
    if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
    {
    	if(tx_buffer.head == tx_buffer.tail)
    	    {
    	      // Buffer empty, so disable interrupts
    	      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

    	    }

    	 else
    	    {
    	      // There is more data in the output buffer. Send the next byte
    	      unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    	      tx_buffer.tail = (tx_buffer.tail + 1) % UART_BUFFER_SIZE;

    	      /******************
    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	      *          sequence: a read operation to USART_SR register followed by a read
    	      *          operation to USART_DR register.
    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	      *          USART_SR register followed by a write operation to USART_DR register.
    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	      *********************/

    	      huart->Instance->SR;
    	      huart->Instance->DR = c;

    	    }
    	return;
    }
}


/*** Deprecated For now. This is not needed, try using other functions to meet the requirement ***/
/*
uint16_t Get_position (char *string)
{
  static uint8_t so_far;
  uint16_t counter;
  int len = strlen (string);
  if (_rx_buffer->tail>_rx_buffer->head)
  {
	  if (Uart_read() == string[so_far])
	  		{
	  		  counter=UART_BUFFER_SIZE-1;
	  		  so_far++;
	  		}
	  else so_far=0;
  }
  unsigned int start = _rx_buffer->tail;
  unsigned int end = _rx_buffer->head;
  for (unsigned int i=start; i<end; i++)
  {
	  if (Uart_read() == string[so_far])
		{
		  counter=i;
		  so_far++;
		}
	  else so_far =0;
  }

  if (so_far == len)
	{
	  so_far =0;
	  return counter;
	}
  else return -1;
}


void Get_string (char *buffer)
{
	int index=0;

	while (_rx_buffer->tail>_rx_buffer->head)
	{
		if ((_rx_buffer->buffer[_rx_buffer->head-1] == '\n')||((_rx_buffer->head == 0) && (_rx_buffer->buffer[UART_BUFFER_SIZE-1] == '\n')))
			{
				buffer[index] = Uart_read();
				index++;
			}
	}
	unsigned int start = _rx_buffer->tail;
	unsigned int end = (_rx_buffer->head);
	if ((_rx_buffer->buffer[end-1] == '\n'))
	{

		for (unsigned int i=start; i<end; i++)
		{
			buffer[index] = Uart_read();
			index++;
		}
	}
}
*/
