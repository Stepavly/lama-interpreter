FLAGS=-m32 -O0 -fstack-protector-all -DDEBUG_VERSION

all: interpreter.o

interpreter.o: interpreter.c
	$(CC) $(FLAGS) -g -c interpreter.c

clean:
	$(RM) *.a *.o *~ interpreter