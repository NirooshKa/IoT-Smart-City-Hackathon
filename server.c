/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.c
 * Author: kammulaa
 *
 * Created on February 20, 2018, 1:48 PM
 */

//Includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

/* -----------WAYS YOU CAN TEST MY SERVER----------------------------
 * i) You can either use telnet (command: tenlet [ip address] [port #]),
 * ii) Use my client.c file to test my server; g++ -o client client.c
 * iii) How to run my program (Unix Terminal): g++ -o server server.c -lpthread
 * iv) (A SIDE NOTE) server.c, client.c and their makefiles must all be in 1 folder before running 
 */

/* ------------COMMON ERRORS I MADE WHILE PROGRAMMING----------------
 * i) Giving strcmp chars instead of pointers/string arrays
 * ii) Forgetting to add \n to make my text look neater
 * iii) Forgetting to cast, forgetting basic pointer arithematic, or not deferencing structs
 */

/*LIST OF FUNCTIONS (xx -> done, x-> needs to be made prettier, but functional)
*xx1. BROADCAST MESSAGE/NEW USER 
*xx2. LIST ALL CLIENTS
*xx3. EXIT
*xx4. SEND MESSAGE TO USER
*xx5. CHECK USERNAME
*xx6. MAIN/SERVER SET UP
*xx7. CREATE MULTIPLE CLIENTS (FORKING)
*xx8. CLEAN UP MEMORY (IF NECESSARY)
*/

/*WHAT TO ADD IF I WERE TO DO THIS HACKATHON AGAIN
* Add extra features such an an execv to run a server without making using any terminal command
* Use a waitpid command to ensure people who don't say anything get kicked out eventually
*/

//*******************************1.GLOBAL VARIABLES*******************************
int maxDataSize = 100;
int numberOfClients = 0; //Number of active clients
int maxClients = 100;
//*****************************************************************************

//***************************2.STRUCTURE FOR THE CLIENT(I should've done a link list, but oh well)**************
struct client
{
    int socketID;
    char* username;
    bool active = false; // This is when the user exits, and there is an empty space.
    pthread_t threadInfo;
}; //Data structure being used: Array, with booleans to determine if the space is a null or not
//*************************************************************************************************************

//**********************3.STRUCTURE TO PASS INTO THE PTHREAD_CREATE FUNCTION********************* 
struct passedArgs
{
    int clientSocket;
    struct client* clients;
    pthread_t threadInfo;
};
//********************************************************************************

//**************************4.LOCKS THROUGHOUT MY ENTIRE PROGRAM*****************
//1. Locks for main function
pthread_mutex_t lockMain1 = PTHREAD_MUTEX_INITIALIZER;

//2. Locks for the username part
pthread_mutex_t userLock0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLock4 = PTHREAD_MUTEX_INITIALIZER;

//3. Locks for the exit function
pthread_mutex_t exitLock1 = PTHREAD_MUTEX_INITIALIZER;
//************************************************************************************


//----------------------- PARSING FUNCTION #1: BROADCAST ----------------------------------
void callBroadcast(int clientIndex, char* message, int argLength, struct client* clients){

    char messageToSend[maxDataSize];
    sprintf(messageToSend, "**NEW PUBLIC MESSAGE FROM**: %s\n------------------------\n%s\n", clients[clientIndex].username,message);
    bool found = false;
    int messagesSent = 0;

    printf("NEW PUBLIC MESSAGE FROM: -> %s\n", clients[clientIndex].username);
    printf("---------------------------------------------\n");
    printf("What the message said: %s\n", message);

    for(int i = 0; i<maxClients && messagesSent<numberOfClients-1; i++){      
      
     	if(i == clientIndex || clients[i].active == false)
        {
            continue;  
        }

        else
        {
            //Broadcast message otherwise
            if(clients[i].active)
            {
                //Send to client: "PUBLIC MESSAGE FROM"
                if ((send(clients[i].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
                {
                    fprintf(stderr, "Error with sending!");   
                    exit(1);
                }

           }
            messagesSent++;
            found = true;
        }
		
    }

    //Preparing what to put into the ack message
    if(found)
    {
        sprintf(messageToSend, "**Broadcasted message completed!**\n");
    }

    else
    {
        sprintf(messageToSend, "**Either the message failed, or nobody else is online.**\n");
    }


    //Send the prepared ack back to the user who sent message.
    if ((send(clients[clientIndex].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
    {
        fprintf(stderr, "Error with sending!");   
    }

}

//------------------- PARSING FUNCTION #1.1: BROADCAST NEW USER ------------------------
void broadcastNewUser(int clientIndex, char* message, int argLength, struct client* clients){

    char messageToSend[maxDataSize];
    sprintf(messageToSend, "**NEW USER**: %s\n----------------------------\n", message);
    int messagesSent = 0;

    printf("\nNEW USER: -> %s", clients[clientIndex].username);
    printf("\n-------------------------------------------------\n\n");

    for(int i = 0; i<maxClients && messagesSent<numberOfClients-1; i++){
		
        if(i == clientIndex || clients[i].active == false)
        {
            continue;  
        }

        else
        {
            //Broadcast message otherwise
            if(clients[i].active)
            {
                //Send to client: "PUBLIC MESSAGE FROM"
                if ((send(clients[i].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
                {
                    fprintf(stderr, "Error with sending!");   
                    exit(1);
                }

           }
            messagesSent++;
        }
		
    }

    sprintf(messageToSend, "userAck");

    //Send the prepared ack back to the user who sent message.
    if ((send(clients[clientIndex].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
    {
        fprintf(stderr, "Error with sending!");   
    }

}

//---------------- PARSING FUNCTION #2: LIST ALL ACTIVE CLIENTS ----------------
void callList(int clientIndex, struct client* clients){
    
    int i = 0, displayed = 0;
    char messageToSend[maxDataSize];
    sprintf(messageToSend, "**Command sent successfully**\n");

    printf("LIST OF USERS relative to: -> %s", clients[clientIndex].username);
    printf("\n-------------------------------------------\n");

    for(i = 0; i<maxClients && displayed<numberOfClients - 1; i++)
    {
        if (i == clientIndex)
            {
                continue;
            }

        else if (clients[i].active)
            {
                printf("%s", clients[i].username);
                displayed++;
            }
    }

    //Send the prepared ack back to the user who asked for command.
    if ((send(clients[clientIndex].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
    {
        fprintf(stderr, "Error with sending!");   
    }
    
    printf("\n");
}

//------------------ PARSING FUNCTION #3: EXIT CURRENT CLIENT ------------------
void callExit(struct client* clients, int clientIndex){

    pthread_mutex_lock(&exitLock1); //START OF CRITICAL SECTION
       
        clients[clientIndex].active = false;
        free(clients[clientIndex].username);
        numberOfClients--;  
       
        //Send an ack to exit
        if ((send(clients[clientIndex].socketID, "**Exited Program**\n", 20, 0)) == -1) 
            {
                fprintf(stderr, "Error with sending!");   
            }

        close(clients[clientIndex].socketID); 

    pthread_mutex_unlock(&exitLock1); //END OF CRITICAL SECTION
        
}

//--------------------- PARSING FUNCTION #4: SEND TO USER ----------------------
bool sendToUser(int clientIndex, char* username, char* message, int argLength, struct client* clients){
    
    char messageToSend[maxDataSize];
    sprintf(messageToSend,"**PRIVATE MESSAGE FROM**: %s\n-----------------------\n%s\n", username,message);

    bool found = false;
    // Finding the user you want to send the message to
    for (int i = 0;  i<maxClients; i++){ 
	
        if(clientIndex != i && clients[i].active && strncmp(clients[i].username, username, strlen(username)) == 0) {

            if ((send(clients[i].socketID, messageToSend, strlen(messageToSend), 0)) == -1) 
            {
                fprintf(stderr, "Error with sending!");   
            }
            found = true;
            break;
			
        }
		
    }

    if(!found) {
        sprintf(messageToSend, "**ERROR! USER DOES NOT EXIST**\n");
    } 

    else {
        sprintf(messageToSend, "**Sent message to**: %s\n", username);
    }

    //Send ack or nack back to client
    if ((send(clients[clientIndex].socketID, messageToSend, strlen(messageToSend), 0)) == -1){
        fprintf(stderr, "Error with sending!");   
    }

	return found;

}

//---------------------- THE MASTER PARSER -----------------------------------

void commandParser(int clientIndex, struct client* clients, int clientSocket){
	
    while(1){
    int numBytes;
    char clientResponse[maxDataSize];
    char command[maxDataSize];
    char message[maxDataSize];
    int i = 0; //For looking at the command
    int j = 0; //For filling up the message
   

    //1. Read the command. Scanf, but from client
    if ((numBytes = recv(clients[clientIndex].socketID, clientResponse, maxDataSize-1, 0)) == -1)
        {
            perror("recv");           
            exit(1);
        }
		
	//1.1. Breaking up/reading the command/
	clientResponse[numBytes] = '\0'; // ALWAYS ADD THIS TO END OF STRING

    //For scanning the command type (stops on any escape character)
    sscanf(clientResponse, "%s ", &command); 

    //For scanning the message (stuff after the command)
    for(i = strlen(command),j=0; i < numBytes; i++, j++) {
		message[j] = clientResponse[i];	
	}

    message[j] = '\0'; 

	//2.****************************PARSER******************************************
        
        //2.1: Parsing function #1
    if(!(strncmp(command,"broadcast",9))){   
		callBroadcast(clientIndex, message, strlen(message), clients);
        }

	   //2.2: Parsing function #2
	else if (!(strncmp(command,"list",4))){
            callList(clientIndex, clients);
        }

	   //2.3: Parsing function #3
	else if (!(strncmp(command,"exit",4))){
            callExit(clients, clientIndex); //Deallocates the created user
            break;//This is to fix the receiving after exiting error
        }

        //2.4: Parsing function #4
    else{
            sendToUser(clientIndex, command, message, strlen(message), clients);
        }

    }
}

//---------------------------ALLOCATING MEMORY TO CREATE A NEW USER-------------------------

int checkUsername(struct client* clients, int clientSocket){

    int numBytes;
    char username[maxDataSize];
    int i = 0, status;

    pthread_mutex_lock(&userLock0); 
		numberOfClients++;
    pthread_mutex_unlock(&userLock0); 

    //Receive the username (BLOCKING CALL #2)
    if ((numBytes = recv(clientSocket, username, maxDataSize-1, 0)) == -1)
    {   
        pthread_mutex_lock(&userLock1);
			perror("recv");
			numberOfClients--;
			close(clientSocket);
        pthread_mutex_unlock(&userLock1);
        return -1;
    }

    //Redeclaring the end point of the array
    username[numBytes] = '\0'; 
    
    //DEALING WITH USERNAME STARTS HERE
    int clientIndex = -1;

    for(int i = 0; i < maxClients && i < numberOfClients; i++)
    {
       if(clients[i].active) {
            if(strcmp(username, clients[i].username) == 0) //Username already exists!
            {  
            pthread_mutex_lock(&userLock2);    
				numberOfClients--;
				close(clientSocket);
            pthread_mutex_unlock(&userLock2);
            return -1;
            }  
       } 

       else if (clientIndex < 0) 
       {
            clientIndex = i;
       }
   }
    
    //Update client upon finding a good position for the client
    pthread_mutex_lock(&userLock3);
		clients[clientIndex].socketID = clientSocket;
		clients[clientIndex].active = true;
		clients[clientIndex].username = (char*)malloc(sizeof(char)*numBytes);
		strncpy(clients[clientIndex].username, username, numBytes);
    pthread_mutex_unlock(&userLock3);
    
    //Acknowledgement sent after a successful username parse (for debugging purposes)
    if ((status = send(clientSocket, "ACKNOWLEDGEMENT", 16, 0)) == -1)
    {   
        pthread_mutex_lock(&userLock4); 
        fprintf(stderr, "Error with sending!"); 
        free(clients[clientIndex].username);
        numberOfClients--;
        close(clientSocket);
        pthread_mutex_unlock(&userLock4);
        return -1;
    }

    if((status = send(clientSocket, "\n", 2, 0)) == -1)
    {
        fprintf(stderr, "Error with sending!!"); //# of exclamation marks = my error flag :p
        close(clientSocket);
        return -1;
    }

    printf("*******************NEW USERNAME DECLARED FROM SOCKET #%d**********************\n", clientSocket);
    printf("Hello There, %s\n\n", username);

    //Broadcasting the username to everyone
    if(numberOfClients > 1){
        broadcastNewUser(clientIndex, username, strlen(username), clients);
    }
    
    return clientIndex;
}

//-------------THE THREADING FUNCTION THAT NEVER ENDS UNTIL USER TYPES "EXIT"------------------
//This is where threads for each client gets made within a process
/***For the thread_create function to work, you must do the following:*****
*1. Make a function that takes in a void* struct of arguments
*2. Make it return a void* value
*3. CAST THE STRUCT IF NECESSARY
*4. Don't worry, you don't have to re-use the struct up after you passed it 
*/
void * handleClientThread(void * args1){
    struct passedArgs* args = (struct passedArgs *) args1; 

    printf("*******************NEW THREAD CREATED**********************\n");
    fprintf(stderr,"Accepted client on socket: %d\n", args->clientSocket);
    fprintf(stderr,"Waiting for client's username\n\n");
    
    //8. This is where parsing the clients' username happens
    int clientIndex = checkUsername((*args).clients, (*args).clientSocket);

    //9. Parsing for the rest of the commands starts here
    if(clientIndex >= 0)
    {
        (*args).clients[clientIndex].threadInfo = (*args).threadInfo;
        commandParser(clientIndex, (*args).clients, (*args).clientSocket);
    }
}

//------------------------------------------------------------------------------
int main(int argc, char** argv){

    //Variables used to set up the internet connection
    int numBytes, storeClient = 0;
    char buf[maxDataSize];
    int backlog = 10; //How many ppl will be waiting 
    int status,serverSocket,clientSocket; 
    struct addrinfo info;
    struct addrinfo *serverInfo, *p;
    struct sockaddr_storage clientAddress;
    socklen_t addr_size;
    char ipstr[INET6_ADDRSTRLEN];
	memset(&info, 0, sizeof(info));

    //Variables used for dealing with the clients that contact with this server
    char clientResponse[maxDataSize];
    struct client clients[100];

    //1. Filling in the information for the socket
    info.ai_family = AF_UNSPEC; //IPv4 or v6
    info.ai_socktype = SOCK_STREAM; //TCP or UDP (TCP in this case)
    info.ai_flags = AI_PASSIVE; //Fills in IP for me, or can be preset

    //2. Filling in ai_flags, addr and addrlen information
    
    // Not using HTTP because port 80 requires admin privileges.
    // Null tells server to look up the IP version (IPv4 or v6)
    if(status = getaddrinfo(NULL, argv[1], &info, &serverInfo)){
        fprintf(stderr, "Error with getaddrinfo!");     
        exit(1);
    }

    //3. Creating a socket starts after getting appropriate address information
    if ((serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1){
         fprintf(stderr, "Error with creating socket!");  
         exit(1);
    }

    //4. Bind to a socket
    if (bind(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0){
        perror("ERROR on binding");              
        exit(1);
    }
    
    //5. Listen on a socket for a new connection
    if ((status = listen(serverSocket, backlog)) == -1){
        fprintf(stderr, "Error with listening: %s", strerror(errno)); 
        exit(1);
    }

    //6. Display the addresses we're connected to (for debugging purposes)
    printf("*******************NETWORK DETAILS**********************\n");
    for(p = serverInfo;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET){//IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }

        else { //IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("%s: %s\n", ipver, ipstr);
    }
	
    printf("\n");//For formatting purposes
    freeaddrinfo(serverInfo); //This frees the memory getaddrinfo allocates from   

    while(1) 
	{ 
	addr_size = sizeof(clientAddress);

    //7. Accept any incoming clients, and make a socket for them (BLOCKING CALL #1)
	   if ((clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddress, &addr_size)) == -1)
            {
            fprintf(stderr, "Error with accepting another socket!"); 
            exit(1);
            }

        if (numberOfClients == maxClients)
            {
            pthread_mutex_init(&lockMain1, NULL);
				close(clientSocket);
				numberOfClients--;
			pthread_mutex_destroy(&lockMain1);
            continue;
            }
       
    struct passedArgs args;
    args.clients = clients;
    args.clientSocket = clientSocket;

    //8. Where making a thread happens (Now we go to the handleClientThread function)
        if(pthread_create(&args.threadInfo , NULL ,  handleClientThread , (void*) &args) < 0)
        {
            perror("Could not create thread\n");
            close(clientSocket);
        } 
    }
    
    //9. Either do thread.join, or free all memory here for preventive measures

    printf("I should never get here\n");
    return (EXIT_SUCCESS);

}