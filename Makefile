
# Microcontroller
MCU=at90s8515

# Objects
PROJECT=avrng-vc
OBJECTS=main.o uart.o fifo.o

# Programs
CC=avr-gcc
OBJCOPY=avr-objcopy
AVRSIZE=avr-size

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(PROJECT).out $(PROJECT).hex
	$(OBJCOPY) --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex $(PROJECT).out $(PROJECT).eep

$(PROJECT).out: $(OBJECTS)
	$(CC) -mmcu=$(MCU) -o $(PROJECT).out -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map,$(PROJECT).map $(OBJECTS)
	$(AVRSIZE) -C --mcu=$(MCU) $(PROJECT).out

.c.o:
	$(CC) -Os -Wall -mmcu=$(MCU) -c $< -o $@

clean:
	rm -f *.o *.out *.map *.hex *~ *.eep *.lock *.fuse *.sig
