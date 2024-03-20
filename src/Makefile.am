ACLOCAL_AMFLAGS = -I m4
CC = @CC@ -Wall
CPP = @CPP@
CFLAGS = @CFLAGS@ -I$(top_srcdir)/include
CPPLAFGS = @CFLAGS@
LIBS = @LIBS@ 
DEFS = @DEFS@ 

##########################################
#			Main Program				 #
##########################################
bin_PROGRAMS = gpioExample

gpioExample_SOURCES = gpioExample.c

clean :
	rm -rf *.o gpioExample
