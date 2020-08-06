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

#ifndef _UART_H_
#define _UART_H_
#define nop() __asm volatile ("nop")
#include "config.h"
#include <avr/io.h>

extern void io_init();

extern void io_write(const uint16_t);

extern uint16_t io_read(int mode);
extern uint16_t uart_getc_nowait();

extern void enable_tx(void);
extern void enable_rx(int mode);
extern void _set_baud(uint16_t baud);
#endif /* _UART_H_ */
