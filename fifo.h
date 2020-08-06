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

#ifndef _FIFO_H_
#define _FIFO_H_

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct
{
	uint8_t volatile count;
	uint8_t size;
	uint16_t *pread;
	uint16_t *pwrite;
	uint8_t read2end, write2end;
} fifo_t;

extern void fifo_init (fifo_t*, uint16_t* buf, const uint8_t size);
extern uint16_t fifo_put (fifo_t*, const uint16_t data);
extern uint16_t fifo_get_wait (fifo_t*);
extern uint16_t fifo_get_nowait (fifo_t*);

static inline uint16_t _inline_fifo_put (fifo_t *f, const uint16_t data)
{
	if (f->count >= f->size) return 0;

	uint16_t * pwrite = f->pwrite;

	*(pwrite++) = data;

	uint8_t write2end = f->write2end;

	if (--write2end == 0)
	{
		write2end = f->size;
		pwrite -= write2end;
	}

	f->write2end = write2end;
	f->pwrite = pwrite;

	uint8_t sreg = SREG;
	cli();
	f->count++;
	SREG = sreg;

	return 1;
}

static inline uint16_t _inline_fifo_get (fifo_t *f)
{
	uint16_t *pread = f->pread;
	uint16_t data = *(pread++);
	uint16_t read2end = f->read2end;

	if (--read2end == 0)
	{
		read2end = f->size;
		pread -= read2end;
	}

	f->pread = pread;
	f->read2end = read2end;

	uint8_t sreg = SREG;
	cli();
	f->count--;
	SREG = sreg;

	return data;
}

#endif /* FIFO_H */
