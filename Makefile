DEBUG= -w -g
EXECS= client server

all:	$(EXECS)

client:  
	g++ -g -Wall -o client client.c

server: 
	g++ -g -Wall -o server server.c -lpthread
	
clean:
	-rm -f $(EXECS) *.o *.~
