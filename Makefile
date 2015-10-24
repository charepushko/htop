all: htop

htop: g++ win.o cfg.o main.o -o htop

clean: 	rm -f *.o htop

