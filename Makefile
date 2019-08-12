# for Windows (mingw)
ifeq ($(OS),Windows_NT)
ifeq ($(MSYSTEM),MINGW32)
CFLAGS	= -g -Wall 
LIBS	= 
# Reference compiled libraries in local directory (32bit)
CFLAGS	+= -I./libserialport/win32/include/
LIBS	+= -L./libserialport/win32/lib/ -lserialport -lsetupapi
# packman -S mingw-w64-i686-json-c (32bit)
LIBS	+= -ljson-c
else
CFLAGS	= -g -Wall 
LIBS	= 
# Reference compiled libraries in local directory (64bit)
CFLAGS	+= -I./libserialport/x64/include/
LIBS	+= -L./libserialport/x64/lib/ -lserialport -lsetupapi
# packman -S mingw-w64-x86_64-json-c (64bit)
LIBS	+= -ljson-c
endif
endif

# for Linux
ifeq ($(shell uname),Linux)
CFLAGS	= -g -Wall 
LIBS	= 
# sudo apt-get install libserialport
CFLAGS	+= $(shell pkg-config --cflags libserialport)
LIBS	+= $(shell pkg-config --libs libserialport)
# sudo apt-get install libjson-c-dev
CFLAGS	+= $(shell pkg-config --cflags json-c)
LIBS	+= $(shell pkg-config --libs json-c)
endif

# for MacOSX
ifeq ($(shell uname),Darwin)
CFLAGS	= -g -Wall 
LIBS	= 
# brew install libserialport
CFLAGS	+= -I$(shell brew --prefix libserialport)/include
LIBS	+= -L$(shell brew --prefix libserialport)/lib -lserialport
# brew install json-c
CFLAGS	+= -I$(shell brew --prefix json-c)/include
LIBS	+= -L$(shell brew --prefix json-c)/lib -ljson-c
endif


irm_cdc_cmd:

irm_cdc_cmd.o:	irm_cdc_cmd.c
	$(CC) $(CFLAGS) -c irm_cdc_cmd.c -o irm_cdc_cmd.o

irm_cdc_cmd:	irm_cdc_cmd.o
	$(CC) $(CFLAGS) irm_cdc_cmd.o -o irm_cdc_cmd $(LIBS) 

clean:
	$(RM) irm_cdc_cmd
	$(RM) irm_cdc_cmd.o
