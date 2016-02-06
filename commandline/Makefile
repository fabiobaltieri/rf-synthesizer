USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs`

BIN = rfsynth
LIB = librfsynth.a

OBJECTS = opendevice.o main.o
LIBOBJECTS = librfsynth.o

CC	= gcc
CFLAGS	= $(CPPFLAGS) $(USBFLAGS) -O -g -Wall
LIBS	= $(USBLIBS) -lm

all: $(BIN) $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(OBJECTS) $(LIB)
	$(CC) -o $(BIN) $(OBJECTS) $(LIBS) $(LIB)

$(LIB): $(LIBOBJECTS)
	ar r $(LIB) $(LIBOBJECTS)

strip: $(BIN)
	strip $(BIN)

clean:
	rm -f *.o $(BIN) $(LIB)
