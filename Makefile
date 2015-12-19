all: htop
htop:
	g++ -std=c++11 main.cpp -o htop -lncurses
clean:
	rm -f *.o htop

