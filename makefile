CFLAGS= -g
ysh: ysh.o cmdscan.o
ysh.o: ysh.c
cmdscan.o: cmdscan.c
clean:
	rm *.o ysh

