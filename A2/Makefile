DEPS = pcb.h

%.o: %.c $(DEPS)
	gcc -c -o $@ $<


mysh: shell.c interpreter.c shellmemory.c
	gcc -c shell.c interpreter.c shellmemory.c readyqueue.c scheduler.c  
	gcc -o mysh shell.o interpreter.o shellmemory.o readyqueue.o scheduler.o

clean: 
	rm mysh; rm *.o
