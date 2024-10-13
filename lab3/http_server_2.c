//  Created by Behnam Dezfouli
//  CSEN, Santa Clara University
//

// This program implements a web server
//
// The input arguments are as follows:
// argv[1]: Sever's port number


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h> // to get file size


#define COMMAND_BUFF    15000       // Size of the buffer used to store the result of command execution
#define SOCK_READ_BUFF  4096        // Size of the buffer used to store the bytes read over socket
#define REPLY_BUFF      20000       // Size of the buffer used to store the message sent to client
#define FILE_READ       10

#define HTML_FILE       "index.html"


// Socket descriptors
int        socket_fd = 0;            // socket descriptor
int        connection_fd = 0;        // new connection descriptor


void  INThandler(int sig)
{
    char  input;
    
    signal(sig, SIG_IGN);
    printf("Did you hit Ctrl-C?\n"
           "Do you want to quit? [y/n] \n");
    
    
    input = getchar();
    input = getchar();
    if (input == 'y'|| input == 'Y'){ //option to kill program
        exit(0);
    }
    
    signal(SIGINT,INThandler);
    
}


// main function ---------------
int main (int argc, char *argv[])
{
    
    // Register a function to handle SIGINT ( SIGNINT is interrupt the process )
    signal(SIGINT, INThandler);
    
    
    int        net_bytes_read;                // numer of bytes received over socket
    struct     sockaddr_in serv_addr;         // Address format structure
    char       net_buff[SOCK_READ_BUFF];      // buffer to hold characters read from socket
    char       message_buff[REPLY_BUFF];      // buffer to hold the message to be sent to the client
    
    char       file_buff[FILE_READ];          // to hold the bytes read from source file
    FILE       *source_file;                  // pointer to the source file
    
    
    // pointer to the file that will include the received bytes over socket
    FILE  *dest_file;
    
    
    if (argc < 2) // Note: the name of the program is counted as an argument
    {
        printf ("Port number not specified!\n");
        return 1;
    }
    
    
    //STUDENT WORK
    //set all variables within structure
    serv_addr.sin_family = AF_INET; //set address family   
    serv_addr.sin_port =  htons(atoi(argv[1])); //setting port # from client and converting to network byte order 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //setting home address 
    
    
    // Create socket
    if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf ("Error: Could not create socket! \n");
        return 1;
    }
    
    // To prevent "Address in use" error
    // The SO_REUSEADDR socket option, which explicitly allows a process to bind to a port which remains in TIME_WAIT
    // STUDENT WORK
    int r = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &r, sizeof(int))<0)
    {
	    perror("setting socket option failed \n");
        close(socket_fd);
	    exit(EXIT_FAILURE);
    }
    
    // **bind it to all interfaces, on the specified port number    
    if (bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
	    perror(" Cannot bind \n");
	    close(socket_fd);
	    exit(EXIT_FAILURE);
    }  
    
    // Accept up to 1 connections
    if (listen(socket_fd, 1) < 0)
    {
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }
    
    printf ("Listening to incoming connections... \n");
    
    
    unsigned int option = 0; // Variable to hold user option
    printf("1: System network configuration \n2: Regular HTTP server\n");
    scanf("%u", &option);
    
    // The web server returns current processor and memory utilization
    if ( option == 1 )
    {
        printf("System network configuration (using ifconfig)... \n");
    }
    // The requested file is returned
    else if (option == 2)
    {
        printf("Regular server (only serving html files)... \n\n");
    }
    else
        printf ("Invalid Option! \n\n");
    
    
    while (1)
    {
        // Accept incoming connection request from client
        if((connection_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1)
        {
            printf("%d\n",connection_fd);
            perror("Cannot accept new socket");
            close (socket_fd);
            return 0;
        }
        
        
        printf ("Incoming connection: Accepted! \n");
        
        memset (net_buff, '\0', sizeof (net_buff));
        
        // Return system utilization info
        if ( option == 1 )
        {
            // run a command -- we run ifconfig here (you can try other commands as well)
            FILE *system_info = popen("ifconfig", "r");
            
            if (system_info == NULL)
            {
                perror("FILE READ ERROR\n");
            }
	        
	        net_bytes_read = (fread(net_buff, 1, sizeof(net_buff), system_info));


            if(net_bytes_read>0)
            {
                snprintf(message_buff, sizeof(message_buff), "HTTP/1.1 200 OK \nContent-Type: text/plain \nContent-Length: %d\n\n", net_bytes_read);

                //connect header with ifconfig output
                strncat(message_buff, net_buff, net_bytes_read);
                write(connection_fd, message_buff, sizeof(message_buff));


                // close the connection
                shutdown (connection_fd, SHUT_RDWR);
                close (connection_fd);
            }
            pclose(system_info);
        }
        else if (option == 2)
        {
            // To get the size of html file
            struct stat sbuf;      /* file status */
            
            // make sure the file exists
            // HTML_FILE is index.html and is statically defined
            if (stat(HTML_FILE, &sbuf) < 0) {
                printf("404 not found");
                // continue
            }
            
            // Open the source file
            if ((source_file = fopen(HTML_FILE, "rb")) == NULL) 
            {
                perror ("Error: could not open the source file!\n");
                snprintf(message_buff, sizeof(message_buff), "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<html><title>My SCU Server Error!</title><body bgcolor=\"ffffff\">\n404: Not Found\n<p>Check if the file exists on your server!</p>\n<hr><em>SCU Student Web Server!</em>\n");
                write(connection_fd, message_buff, strlen(message_buff));
                continue;

            }
            else
            {
                snprintf(message_buff, sizeof(message_buff), "HTTP/1.1 200 OK\nServer: SCU COEN Web Server\nContent-Type: text/html\nContent-Length: %d\n\n", (int)sbuf.st_size);

                // Concatenate file contents to message_buff
                // Read and send the file contents
                while ((net_bytes_read = fread(file_buff, sizeof(char), FILE_READ, source_file)) > 0) {
                    strncat(message_buff, file_buff, net_bytes_read); 
                }
                
                // success copied 
                printf("message copied\n");
                
                //write the message into the socket
                write(connection_fd, message_buff, sizeof(message_buff));
                
                
                // Close the file and the connection
                fclose(source_file);
                printf("Reply sent to the client!\n");
            }
            
            shutdown (connection_fd, SHUT_RDWR);
            close (connection_fd);
        }
    }
    
    close (socket_fd);
}
