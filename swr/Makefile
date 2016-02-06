USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs`

BIN = swr
EXTRALIBS = ../commandline/librfsynth.a ../../rf-power-detector/commandline/librfpower.a

OBJECTS = opendevice.o main.o

CC	= gcc
CFLAGS	= $(CPPFLAGS) $(USBFLAGS) -O -g -Wall
LIBS	= $(USBLIBS) $(EXTRALIBS) -lm

all: $(BIN)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(OBJECTS)
	$(CC) -o $(BIN) $(OBJECTS) $(LIBS)

strip: $(BIN)
	strip $(BIN)

clean:
	rm -f *.o $(BIN)
