CC=gcc

all: gc_runtime.o runtime.o virt_stack.o
	ar rc runtime.a gc_runtime.o runtime.o virt_stack.o

gc_runtime.o: gc_runtime.s
	$(CC) -g -fstack-protector-all -m32 -c gc_runtime.s

runtime.o: runtime.c runtime.h
	$(CC) -g -fstack-protector-all -m32 -c runtime.c 

virt_stack.o: virt_stack.h virt_stack.c
	$(CC) -g -fstack-protector-all -m32 -c virt_stack.c

clean:
	$(RM) *.a *.o *~