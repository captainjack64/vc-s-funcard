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

#define F_CPU 3570000UL

/* ATR baud */
#define ATR_BAUDRATE 9600

/* VC baud */
#define VC_BAUDRATE 19200

/* Ratio between ATR baud and VC baud */
#define RATIO (VC_BAUDRATE / ATR_BAUDRATE)

/* Transition delays between transmit and receive */
#define RX_DELAY_ATR 100
#define RX_DELAY_VC  (RX_DELAY_ATR / RATIO)

/* General delay */
#define STD_DELAY (1 / RATIO)

/* Delay for 9600 baud */
#define DELAY_ATR 100

/* Delay for VC */
#define DELAY_VC  (DELAY_ATR / RATIO)

/* Define operating modes */
#define MODE_ATR  0
#define MODE_VC   1

#define _9N1 1 /* 8N1 = 0 / 9N1 = 1 */
