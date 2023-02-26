.PHONY: all clean

NAME_PROGRAM = main

LIBRARIES := ../lib

INCLUDE := ../include

all: $(NAME_PROGRAM).prg

clean:
	rm -f *.prg *.o $(NAME_PROGRAM).s

FILES := \
	$(NAME_PROGRAM).c \
	clearscreen.s \
	$(LIBRARIES)/libX16.lib

$(NAME_PROGRAM).prg: $(FILES)
	cc65 -t cx16 --cpu 65C02 -I $(INCLUDE) -Osir -Cl $(NAME_PROGRAM).c
	ca65 -t cx16 --cpu 65C02 $(NAME_PROGRAM).s
	ca65 -t cx16 --cpu 65C02 clearscreen.s -o clearscreen.o
	ca65 -t cx16 --cpu 65C02 gal.s -o gal.o
	ld65 -t cx16 -L $(LIBRARIES) -o $(NAME_PROGRAM).prg $(NAME_PROGRAM).o clearscreen.o gal.o libX16.lib cx16.lib

run:
	x16emu -prg $(NAME_PROGRAM).prg -run
