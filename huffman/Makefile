cc = gcc
target = mp3_20191251

target : main.o
	$(cc) -o $(target) main.o
main.o : main.c
	$(cc) -c main.c
clean:
	rm *.o $(target)

