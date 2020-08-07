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

#include "uart.h"
#include "fifo.h"

#include <avr/interrupt.h>
#include <util/delay.h>

/* TXD */
#define SUART_TXD_PORT PORTB
#define SUART_TXD_DDR  DDRB
#define SUART_TXD_BIT  PB6


/* RXD */
#define SUART_RXD_PORT PORTB
#define SUART_RXD_PIN  PINB
#define SUART_RXD_DDR  DDRB
#define SUART_RXD_BIT  PB6

static volatile uint16_t outframe;
static volatile uint16_t inframe;
static volatile uint16_t inbits, received;


#define INBUF_SIZE 4
static uint16_t inbuf[INBUF_SIZE];
fifo_t infifo;

void _set_baud(uint16_t baud)
{
	uint8_t tifr = 0;
	cli();
	OCR1A = (uint16_t) ((uint32_t) F_CPU/baud);
	tifr  |= (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A);
	outframe = 0;
	TIFR = tifr;
	sei();
}


void io_init()
{
	uint8_t sreg = SREG;
	cli();
	TCCR1A = 0;
	TCCR1B = (1 << CTC1) | (1 << CS10) | (0 << ICES1) | (1 << ICNC1);
	SREG = sreg;
	fifo_init (&infifo,   inbuf, INBUF_SIZE);
}


void enable_tx(void)
{
	cli();
	/* DISABLE ICP INTERRUPT */
	TIMSK &= ~(1 << TICIE1);
	/* SET PIN TO OUTPUT */
	SUART_TXD_PORT |= (1 << SUART_TXD_BIT);
	SUART_TXD_DDR  |= (1 << SUART_TXD_BIT);
	sei();
}

void enable_rx(int mode)
{
    mode ? _delay_us(RX_DELAY_VC) : _delay_us(RX_DELAY_ATR);
	cli();
	/* SET PIN TO INPUT */
	SUART_RXD_DDR  &= ~(1 << SUART_RXD_BIT);
	SUART_RXD_PORT &= ~(1 << SUART_RXD_BIT);
	/* ENABLE ICP INTERRUPT */
	TIMSK |= (1 << TICIE1) | (1 << TOIE0);
	sei();
}

void io_write(const uint16_t c)
{
	/* WAIT FOR LAST CHAR SENT */
	do
	{
		sei(); nop(); cli(); // yield();
	} while (outframe);

	outframe = (3 << (9 + _9N1)) | (((uint16_t) c) << 1);

	TIMSK |= (1 << OCIE1A);
	TIFR   = (1 << OCF1A);

	sei();
}

/* TX INT */
ISR (TIMER1_COMPA_vect)
{
	uint16_t data = outframe;

	if (data & 1)	
    {
        SUART_TXD_PORT |=  (1 << SUART_TXD_BIT);
    }
	else
    {
        SUART_TXD_PORT &= ~(1 << SUART_TXD_BIT);
    }

	if (1 == data)
	{
		TIMSK &= ~(1 << OCIE1A);
	}

	outframe = data >> 1;
}

/* RX INT */
ISR (TIMER1_CAPT_vect)
{
	uint16_t icr1  = ICR1;
	uint16_t ocr1a = OCR1A;

	uint16_t ocr1b = icr1 + ocr1a/2;
	if (ocr1b >= ocr1a)
		ocr1b -= ocr1a;
	OCR1B = ocr1b;

	TIFR = (1 << OCF1B);
	TIMSK = (TIMSK & ~(1 << TICIE1)) | (1 << OCIE1B);
	inframe = 0;
	inbits = 0;
}

/* FETCH INPUT BITS */
ISR (TIMER1_COMPB_vect)
{

	uint16_t data = inframe >> 1;

	if (SUART_RXD_PIN & (1 << SUART_RXD_BIT))
		data |= (1 << (9 + _9N1));

	uint16_t bits = inbits+1;

	if (10 + _9N1 == bits)
	{
		if ((data & 1) == 0)
			if (data >= (1 << (9 + _9N1)))
			{
				_inline_fifo_put (&infifo, data >> 1);
				received = 1;
			}
		TIMSK = (TIMSK & ~(1 << OCIE1B)) | (1 << TICIE1);
		TIFR = (1 << ICF1);
	}
	else
	{
		inbits = bits;
		inframe = data;
	}
}

uint16_t io_read(int mode)
{
	enable_rx(mode);
	return (uint16_t) _9N1 ? fifo_get_wait(&infifo) & 0x1FF : fifo_get_wait(&infifo) & 0xFF;
}

uint16_t uart_getc_nowait()
{
	return fifo_get_nowait (&infifo);
}
