CC=C:\TDM-GCC-64\bin\gcc.exe
CFLAGS=-Wall -Wextra --std=c99 -Iinclude -IC:\SDL2-2.0.4\x86_64-w64-mingw32\include -DSLOWDOWN_ENABLED
LDFLAGS=-LC:\SDL2-2.0.4\x86_64-w64-mingw32\lib -lmingw32 -lSDL2main -lSDL2 -lpthread
LIBS=gb_init.o gb_cpu.o gb_cartridge.o graphics.o

# gbed is just gbe with debug flag
all: gbe.exe gbed.exe

gbed.exe: CFLAGS+=-DDEBUG
gbed.exe: main.c $(LIBS)
	$(CC) $(CFLAGS) -o gbed.exe $+ $(LDFLAGS)

gbe.exe: main.c $(LIBS)
	$(CC) $(CFLAGS) -o gbe.exe $+ $(LDFLAGS)

clean:
	del *.o gbe