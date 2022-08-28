# WinAVR cross-compiler toolchain is used here
CC = avr-gcc
OBJCOPY = avr-objcopy

# If you are not using ATtiny2313 and the USBtiny programmer, 
# update the lines below to match your configuration
CFLAGS = -Wall -Os -I usbdrv -mmcu=atmega328p
OBJFLAGS = -j .text -j .data -O ihex

SRC=main

# Object files for the firmware (usbdrv/oddebug.o not strictly needed I think)
OBJECTS = usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o $(SRC).o

all: $(SRC).hex 

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

# Main.elf requires additional objects to the firmware, not just main.o
$(SRC).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Without this dependance, .o files will not be recompiled if you change 
# the config! I spent a few hours debugging because of this...
$(OBJECTS): usbdrv/usbconfig.h

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

# for uncompatino
flush:
	sudo ~/src/avrdude-5.8/avrdude -c diecimila -C ~/src/avrdude-5.8/avrdude.conf -p atmega328p -e -U flash:w:$(SRC).hex -P ft0 -v -v

