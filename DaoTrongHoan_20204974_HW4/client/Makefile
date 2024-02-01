CC = gcc

udp_server: udp_client.o
	$(CC) udp_client.o -o client

udp_client.o: udp_client.c
	$(CC) -c udp_client.c

clean: 
	$(RM) udp_client *.o *~ 