IDIR =../include
CC=clang++
C=clang

MAIN_FLAGS=-O0 -I$(IDIR) -g `mysql_config --include`
CPP_FLAGS=-std=c++11 $(MAIN_FLAGS) #-ferror-limit=200

CFLAGS=$(MAIN_FLAGS)

ODIR=obj
LDIR =../lib

LIBS= -lpthread -ldl -lssl -lcrypto `mysql_config --libs`

_DEPS = main.h _ssl.h database.h file.h stream.h proxy.h server.h log.h config.h err.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = err.o database.o config.o _ssl.o sql_connect.o ssl_stream.o file_stream.o log.o server.o proxy.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


#$(ODIR)/%.o: %.c $(DEPS)
#	$(C) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPP_FLAGS)

dossier.out: $(OBJ)
	$(CC) -o $@ $^ $(CPP_FLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
