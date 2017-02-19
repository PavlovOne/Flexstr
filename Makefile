LIBDIR=./lib
INCDIR=./inc
INCL = -I$(INCDIR)

OBJS = main.o utils.o ./encode/encode.o ./json/jsmn.o ./json/json.c
CC = cc
LD = $(CC)
CFLAGS = -c $(INCL)

LDFLAGS = -lpthread $(LIBDIR)/libmercuryapi.a $(LIBDIR)/libltkc.a $(LIBDIR)/libltkctm.a

all: flexstr clean

everything:
	make all

flexstr: $(OBJS)
	$(LD) -o flexstr $^ $(LDFLAGS)

%.o: %.c ./encode/%.c ./json/%.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o ./encode/*.o ./json/*.o

cleanest:
	make clean
	rm -f flexstr