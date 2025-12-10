
linux: font_16_32.o font_8_16.o
	cc -c src/hud_main.c -ggdb -Wall -odrn_hub_linux.o
	cc drn_hub_linux.o font_8_16.o font_16_32.o  -lraylib -lm  -odrn_hub_linux

font_8_16.o:
	cc -c fonts/font_8_16.c -ofont_8_16.o

font_16_32.o:
	cc -c fonts/font_16_32.c -ofont_16_32.o


clean:
	rm -f drn_hub_linux
	rm -f drn_hub_linux.o
	rm -f font_8_16.o
	rm -f font_16_32.o