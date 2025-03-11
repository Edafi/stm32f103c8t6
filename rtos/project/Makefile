######################################################################
#  Project Makefile
######################################################################

BINARY		= main
SRCFILES = 	main.c rtos/heap_4.c rtos/list.c rtos/port.c rtos/tasks.c rtos/opencm3.c rtos/queue.c \
			libraries/Hardware/enc28j60.c /libraries/uIP-1.0/uip/uip.h


LDSCRIPT	= stm32f103c8t6.ld

include ../../Makefile.incl
include ../Makefile.rtos

# Правило для сборки main.elf
main.elf: $(OBJS) $(LDSCRIPT)
	$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) $(TGT_CFLAGS) -o main.elf
	$(SIZE) main.elf

######################################################################
#  NOTES:
#
#	1. remove any modules you don't need from SRCFILES
#	2. "make clean" will remove *.o etc., but leaves *.elf, *.bin
#	3. "make clobber" will "clean" and remove *.elf, *.bin etc.
#	4. "make flash" will perform:
#	   st-flash write main.bin 0x8000000
#
######################################################################