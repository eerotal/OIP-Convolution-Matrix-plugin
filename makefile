CC=gcc
CFLAGS=-Wall -Wpedantic -std=gnu99 -fpic -shared
LFLAGS=-lm -lfreeimage -loipimgutil
NAME=convolution

SRCDIR=src
BUILDDIR=bin
OIPDIR=/home/eero/projects/OIP/src

INCLUDES=-I$(OIPDIR)/imgutil/ -I$(OIPDIR)/headers
LIBS=-L$(OIPDIR)/imgutil/bin/

compile: $(SRCDIR)/*.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(SRCDIR)/*.c -o $(BUILDDIR)/lib$(NAME).so $(INCLUDES) $(LIBS) $(LFLAGS)
