#============================================================
# Name: tiny-bird Makefile
# www.anyma.ch
#============================================================
# Edit the following lines to reflect your paths
#============================================================

#------------------ ATtiny 13 @ 128kHz
DEVICE     	= attiny85
F_CPU		= 128000

PROGRAMMER 	= -c usbtiny -P usb

#### arduino versuche: gehn noch nicht
#PROGRAMMER 	= -c avrisp -P /dev/tty.usbserial-A7006QJg
#PROGRAMMER 	= -c avrisp -P /dev/tty.usbmodem621


# set fuses
#FUSEOPT 	= -U lfuse:w:0x6a:m -U hfuse:w:0xff:m		# Default!
#FUSEOPT 	= -U lfuse:w:0x7a:m -U hfuse:w:0xff:m		# 9.6MHz, ckdiv8 NO, +64ms
#FUSEOPT 	= -U lfuse:w:0x65:m -U hfuse:w:0xff:m		# 4.8MHz, ckdiv8 YES, +4ms
#FUSEOPT 	= -U lfuse:w:0x75:m -U hfuse:w:0xff:m		# 4.8MHz, ckdiv8 NO, +4ms
#FUSEOPT 	= -U lfuse:w:0x67:m -U hfuse:w:0xff:m		# 128kHz, ckdiv8 YES, +4ms
#FUSEOPT 	= -U lfuse:w:0x77:m -U hfuse:w:0xff:m		# 128kHz, ckdiv8 NO, +4ms
# FUSEOPT = -U lfuse:w:0x7B:m -U hfuse:w:0xff:m		# 128kHz, ckdiv8 NO, +64ms
# FUSEOPT = -U lfuse:w:0x54:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m	# 128kHz, ckdiv8 YES, +4ms 
FUSEOPT = -U lfuse:w:0xd4:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m	# 128kHz, ckdiv8 NO, +4ms 



AVRDUDE 	= avrdude $(PROGRAMMER) -p $(DEVICE) -F -B50
# Choose your favorite programmer and interface above.


COMPILE = avr-gcc -Wall -Os -I. -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)#-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.
# F_CPU is already defined in .c file?

OBJECTS = build/main.o


# symbolic targets:
all:	build/main.hex


#===============================================================
# generic make file to be included by the project's own makefile

build: 
	mkdir build


build/%.o: %.c build
	$(COMPILE) -c $< -o $@


flash:	all
	$(AVRDUDE) -U flash:w:build/main.hex:i

clean:
	rm -rf build

# file targets:
build/main.bin:	$(OBJECTS)
	$(COMPILE) -o $@ $(OBJECTS)

build/main.hex:	build/main.bin
	rm -f $@ build/main.eep.hex
	avr-objcopy -j .text -j .data -O ihex $< $@
	avr-size $@
disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c

fuse:
	$(AVRDUDE) $(FUSEOPT)

check: 
	$(AVRDUDE) -v
