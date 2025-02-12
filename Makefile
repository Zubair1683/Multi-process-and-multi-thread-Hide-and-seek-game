multithreads: game_threads_group8.c
	gcc game_threads_group8.c -o multithreads -pthread

multiprocesses: game_processes_group8.c
	gcc game_processes_group8.c -o multiprocesses
	
.PONY: clean

all: multithreads multiprocesses

clean:
	rm -f multithreads
	rm -f multiprocesses
