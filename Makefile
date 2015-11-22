all: htop
htop:
	g++ main.cpp -o htop
clean:
	rm -f *.o htop

