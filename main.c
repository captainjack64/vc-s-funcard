/* avrng - AVR-based Videocrypt card firmware for hacktv                 */
/*=======================================================================*/
/* Copyright 2019 Marco Wabbel <marco@familie-wabbel.de>                 */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*                                                                       */
/* Original Copyright for the base of this Firmware goes to:             */
/*              Philip Heron <phil@sanslogic.co.uk>                      */

/* Hardware setup:
 *
 * F_CPU is 3.57 MHz.
 * PB6 is I/O, 9-bit software serial.
 * Timer1 is used to drive I/O on PB6 at x3 actual rate.
 * Interrupt Timing = F_CPU / (BAUDRATE * 3)
 * Communication is 8-ODD-1 so 9 Bit UART is needed
*/

#include "config.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include <string.h>

#define INS_SERIAL 0x70
#define INS_CW     0x74
#define INS_AUTH   0x76
#define INS_SEED   0x78
#define INS_MSG    0x7A
#define INS_MBOX   0x7C

/* Standard responses */
/* Videocrypt 1 ATR   */
const uint8_t _atr_vc1[] PROGMEM = { 0x3f, 0xfa, 0x11, 0x25, 0x05, 0x00, 0x01, 0xb0, 
									 0x02, 0x3b, 0x36, 0x4d, 0x59, 0x02, 0x80, 0x81 };
/* Videocrypt S ATR ?? */
const uint8_t _atr_vcs[] PROGMEM = { 0x3f, 0xfa, 0x12, 0x25, 0x05, 0x00, 0x01, 0xb0, 
									 0x02, 0x3b, 0x36, 0x4d, 0x59, 0x02, 0x80, 0x81 };
									
const uint8_t  _serial[] PROGMEM = { 0x70, 0x2b, 0x02, 0x56, 0x02, 0x03, 0x04 };

const uint8_t  _ans_7c[] PROGMEM = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
									 
const uint8_t  _ans_78[] PROGMEM = { 0x78, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

const uint8_t  _ans_7a[] PROGMEM = { 0x7A, 0x80, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
									 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
									 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
const uint8_t     _ack[] PROGMEM = { 0x90, 0x00 };

/* Some helpers */
int mode;

/* I/O buffers */
static uint8_t _ib[32];

uint16_t _get_rx_delay(int mode)
{
	return (mode ? RX_DELAY_VC : RX_DELAY_ATR);
}

uint16_t _get_delay(int mode)
{
	return (mode ? DELAY_VC : DELAY_ATR);
}

uint16_t _get_baud(int mode)
{
	return (mode ? VC_BAUDRATE : ATR_BAUDRATE);
}

uint8_t _parity(uint8_t p)
{
	p = p ^ (p >> 4 | p << 4);
	p = p ^ (p >> 2);
	p = p ^ (p >> 1);
	return p & 1;
}

uint8_t _rev(uint8_t xx)
{
	xx =((xx & 0xaa) >> 1) | ((xx & 0x55) << 1);
	xx =((xx & 0xcc) >> 2) | ((xx & 0x33) << 2);
	xx =((xx & 0xf0) >> 4) | ((xx & 0x0f) << 4);
	xx ^= 0xFF;

	return xx;
}

void _send_response(const uint8_t *_data, size_t len)
{
	int a;
	enable_tx();
	
	uint8_t c = 0;
	for(a = 0; a < len; a++)
	{
		c = _rev(pgm_read_byte(&_data[a]));
		io_write( _parity(c) ? 0x00 | c: 0x100 | c );
		mode ? _delay_us(DELAY_VC) : _delay_us(DELAY_ATR);
	}
	
	_delay_ms(STD_DELAY);
	enable_rx(mode);
}

void _send_byte(const uint8_t byte)
{
	uint8_t c;
	
	enable_tx();
	
	c = _rev(byte);
	io_write( _parity(c) ? 0x00 | c: 0x100 | c );
	
	_delay_ms(STD_DELAY);
	enable_rx(mode);
}

void _command(void)
{
	int i, len, cmd;
	
	len = _ib[4] + 1;
	cmd = _ib[1];
	
	/* Check command byte */
	switch(cmd)
	{
	/* Answer with seed */
	case INS_SERIAL:
		_send_response(_serial, len);
		break;
	case INS_SEED:
		_send_response(_ans_78, len);
		break;
	case INS_MSG:
		_send_response(_ans_7a, len);
		break;
	case INS_MBOX:
		_send_response(_ans_7c, len);
		break;
	
	/* Receive 32 bytes */
	case INS_CW:
		_send_byte(cmd);
		for(i = 0; i < 32; i++)
		{
			_ib[i] = _rev(io_read(mode) & 0xff);
		}
	
	default:
		break;
	}

	_send_response(_ack, 2);
	}

int main(void)
{
	uint8_t i;

	io_init();
	mode = MODE_ATR;
	
	/* Set baud to 9600 */
	_set_baud(_get_baud(mode));

	/* Enable interrupts */
	sei();
	
	/* Send ATR - always sent in 9600 */
	_send_response(_atr_vcs, 16);
	
	mode = MODE_VC;
	/* Set baud to VC */
	_set_baud(_get_baud(mode));

	while(1)
	{
		/* Wait for CLA byte */
		while(_ib[0] != 0x53)
		{
			_ib[0] = _rev(io_read(mode) & 0xff);
		}

		/* Receive 4 bytes */
		for(i = 1; i < 5; i++)
		{
			_ib[i] = _rev(io_read(mode) & 0xff);
		}
		
		_command();
		
		/* Zeroise buffer */
		memset(_ib, 0, 32);
	}

	return(0);
}
