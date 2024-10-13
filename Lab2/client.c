//
//  Behnam Dezfouli
//  CSEN, Santa Clara University
//

// This program implements a client that connects to a server and transfers the bytes read from a file over the established connection
//
// The input arguments are as follows:
// argv[1]: Sever's IP address
// argv[2]: Server's port number
// argv[3]: Source file
// argv[4]: Destination file at the server side which includes a copy of the source file


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define    SIZE 10






int main (int argc, char *argv[])
{
    int        sockfd = 0;              // socket descriptor
    char       net_buff[SIZE];          // to hold the bytes read from socket
    char       file_buff[SIZE];         // to hold the bytes read from source file
    struct     sockaddr_in serv_addr;   // server address structure
    int        net_bytes_read;          // numer of bytes received over socket
    FILE       *source_file;            // pointer to the source file
    
    if (argc < 5)
    {
        printf ("Usage: ./%s <server ip address> <server port number> <source file>  <destination file>\n", argv[0]);
        return 1;
    }
    
    // creating socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }


    // set server address, initial = 0
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("address invalid");
        exit(EXIT_FAILURE);
    }


    // connection
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection invalid");
        exit(EXIT_FAILURE);
    }
    

    // open source file
    source_file = fopen(argv[3], "rb");
    if (source_file == NULL) {
        perror("error opening source file");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
/*
    // Read data from the source file and send it over the socket
    while ((net_bytes_read = fread(file_buff, 1, SIZE, source_file)) > 0) {
        ssize_t bytes_sent = send(sockfd, file_buff, net_bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Error sending data over socket");
            fclose(source_file);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
*/
    // Read data from the source file and send it over the socket
    while ((net_bytes_read = fread(file_buff, 1, SIZE, source_file)) > 0) {
        ssize_t bytes_written = write(sockfd, file_buff, net_bytes_read);
        if (bytes_written < 0) {
            perror("Error writing data to socket");
            fclose(source_file);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }


    // Close the file and socket
    fclose(source_file);
    close(sockfd);


    
    return 0;
}

