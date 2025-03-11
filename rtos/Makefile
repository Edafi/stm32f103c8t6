######################################################################
#  Makefile for top level rtos subdirectory
######################################################################

PROJS = blinky blinky2 timer1 timer2 uart uart2 uart3 usbcdc usbbulk i2c-pcf8574 \
		winbond rtc rtc2 uart_rxint lab1 lab2 lab3 lab4 lab5 project

all:	libwwg 
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj ; \
	done

clean:	
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clean ; \
	done
	$(MAKE) -$(MAKEFLAGS) -C ./libwwg clean

clobber:
	for proj in $(PROJS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clobber ; \
	done
	$(MAKE) -$(MAKEFLAGS) -C ./libwwg clean

# End
