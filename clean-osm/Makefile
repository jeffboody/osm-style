TARGET   = clean-osm
CLASSES  = osm_parser osm_element osm_log
SOURCE   = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS  = $(TARGET).o $(CLASSES:%=%.o)
HFILES   = $(CLASSES:%=%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I.
LDFLAGS  = -Llibexpat/expat/lib -lexpat -lm -lz
CCC      = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS) libexpat
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: libexpat

libexpat:
	$(MAKE) -C libexpat/expat/lib

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C libexpat/expat/lib clean
	rm libexpat

$(OBJECTS): $(HFILES)
