CC = gcc

udp_server: udp_server.o
	$(CC) udp_server.o -o server

udp_server.o: udp_server.c
	$(CC) -c udp_server.c

clean: 
	$(RM) udp_server *.o *~ 