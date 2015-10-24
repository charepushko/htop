all: htop

htop: g++ screen.o -o htop

clean: 	rm -f *.o htop

