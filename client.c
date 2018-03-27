/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client.c
 * Author: kammulaa
 *
 * Created on February 20, 2018, 3:13 PM
 */

//Includes
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//Define Statements
#define STDIN 0  // file descriptor for standard input

int main(int argc, char** argv) {

    //Variables used to set up the internet connection
    int maxDataSize = 100;
    char buf[maxDataSize];
    int socketClient, numBytes;
    int status,sockfd;
    struct addrinfo info;
    struct addrinfo *serverInfo, *p;
    char ipstr[INET6_ADDRSTRLEN];

    //Variables used for command parsing
    char commandToParse[maxDataSize];
    char messageToBroadcast[maxDataSize];
    char messageToSend[maxDataSize];
    char username[maxDataSize];
    char command[maxDataSize];
    
    //Variables used for my multi client select
    fd_set readfds;
    int fdCount;
    int listener;

    //1: Filling in the information for the socket
    info.ai_family = AF_UNSPEC; //IPv4 or v6
    info.ai_socktype = SOCK_STREAM; //TCP or UDP (TCP in this case)
    memset(&info, 0, sizeof(info));	

    //1.1: This is for when someone types in an invalid command
    if (argc != 4)
        { 
            //The arguments required when running this program
	        fprintf(stderr, "Usage: %s [server_address] [server_port] [user_name]\n", argv[0]);
            exit(1);
        }

    //2: Getting information to feed the socket :P
    if((status = getaddrinfo(argv[1], argv[2], &info, &serverInfo)))
        {
            fprintf(stderr, "Error!");  
            return 1;
        }

    //3: Creating a socket to connect to the server
    for(p = serverInfo; p; p = p->ai_next)
    {    

	   if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))== -1)
            {
                perror("Error connecting to server");   
                continue;
            }

	   if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
            {
                close(sockfd); 
                perror("client: connect");   
                continue;
            }

	   break;//Exits the for loop as we created socket and connected to server
    
    }
    
    //4: For good practice, it's best if we set up everything right after we make the socket
    FD_ZERO(&readfds);
    
    //Make sure stdin isn't blocking
    int flag = fcntl(STDIN, F_GETFL, 0);
    flag = O_NONBLOCK;
    fcntl(STDIN, F_SETFL, flag);

    FD_SET(STDIN, &readfds);
    FD_SET(3, &readfds);

    //4.1: For case where both IPv6 and IPv4 fail the if statement
    if (!p)   
        {
            fprintf(stderr,"Failed to create socket"); 
            return 2;
        }

    //4.2: Delay that makes the program look more realistic
    for(int i = 0; i < 100000000; i++)
        {

        }

    //5: This is where sending the username happens
    if((numBytes = send(sockfd, argv[3], strlen(argv[3]), 0)) ==-1)
	    {
            perror("Error sending message (type 1)");         
            exit(1);
        }
    
    
  	//6: Printing out where and how many bytes I sent
    printf("Sent %d bytes to %s\n", numBytes, argv[1]);	

    //7: Receive an "ok" signal from server. The recv info gets stored in buf
    if ((numBytes = recv(sockfd, buf, maxDataSize-1, 0)) == -1) 
        { 
            perror("Error: Ok signal not obtained.");          
            exit(1);
        }    

    //7.2: Checking if you get an exit signal due to trying to use another person's username
    if(strncmp(buf,"exit",4) == 0)
        {
            memset(buf,'\0', strlen(buf));
            printf("**ERROR: Username Already Taken**\n");
            freeaddrinfo(serverInfo);
            close(sockfd);
            return (EXIT_SUCCESS);
        }

    //Prints the acknowledgement to the client
    printf("**%s RECEIVED FROM SERVER**\n\n", buf);
    memset(buf,'\0', strlen(buf));

    //************************************ACKNOWLEDGEMENTS/USER CHECKS ARE ALL DONE!***************************************
    //************************************SELECT STATEMENTS/COMMAND PARSING STARTS HERE************************************
    while(1) 
    	{
    	
    	//This is where you wait for either STDIN or the receiving buffer to get a command
    	listener = select(2, &readfds, NULL,NULL,NULL);
    		
        if (listener == -1)
        {
        	perror("Error sending message (type 2)");     
	        exit(1);
        }

        //Case 1: Person types into terminal, and the command gets sent
    	if(FD_ISSET(STDIN, &readfds)) 
    	{
    		fgets(command, maxDataSize, stdin);

	    	//2. Send command to server
	    	if((numBytes = send(sockfd, command, strlen(command)+1, 0)) ==-1)
	            {
	                perror("Error sending message (type 2)");     
	                exit(1);
	            }

	         if(strncmp(command, "exit", 4) == 0)
	         	{
	         		freeaddrinfo(serverInfo);
    				close(sockfd); 
    				return (EXIT_SUCCESS);
	         	}   


    	}

    	printf("%d, %d \n", sockfd, listener);

    	//Case 2: After sending, the buffer gets triggered asynchronously, and we get a message
    	if(FD_ISSET(3, &readfds)) 
    	{
    		printf("Entered sockfd\n");

    		if ((numBytes = recv(sockfd, buf, maxDataSize-1, 0)) == -1)
            {
                perror("Error: ok signal not obtained.");     
                exit(1);
            }
 
	        buf[numBytes] = '\0';
	        fprintf(stderr,"%s\n",buf);
    	}

    }

    //Freeing the getaddrinfo malloc call
    freeaddrinfo(serverInfo);
    close(sockfd); 
    return (EXIT_SUCCESS);

}
