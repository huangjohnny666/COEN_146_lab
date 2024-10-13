// SCU COEN 146
//
// This program implements the server side of stop & wait protocol
// the server receives file contents from a client
//
//
// For the stop and wait protocol, the assumestions are:
//      -- packet corruption, and packet loss


#include	"tfv1.h"
#include    <stdlib.h>

// global variables
int state = 0; // we only have two states: 0 and 1
int sock;
struct sockaddr_in serverAddr;
socklen_t addr_size;


// list of functions
int main (int, char *[]);
int my_receive (PACKET *);
int calc_checksum(char *, int); // char is the data, int is count of bytes



int main (int argc, char *argv[])
{
	FILE	*fp; // file ptr
	int		n;   // 
	PACKET	buf; // packet


    if (argc != 2)
    {
        printf ("need the port number\n");
        return 1;
    }


    // STUDENT WORK 
	// create socket
	if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf ("socket error\n");
		return 1;
	}
	// Set up server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0)
	{
		printf ("bind error\n");
		return 1;
	}

    // NOTE: this program uses UDP socket, so there is no need to listen to incoming connections!
    
	// error check:
	printf("Server listening on port %s...\n", argv[1]);

	// receive name of the file
    // my_receive() function ensures the received data chunks are not corrupted
	if ((n = my_receive (&buf)) <= 0)
	{
		printf ("could not get the name of the file\n");
		return 1;
	}
    printf ("File name has been received!\n");
    

	// open file
	if ((fp = fopen (buf.data, "wb")) == NULL)
	{
		printf ("error fopen\n");
		return 1;
	}

    
	printf ("Receiving file %s ... \n", buf.data);
    // my_receive() function ensures the received data chunks are not corrupted
	while ((n = my_receive (&buf)) > 0)
	{
		printf ("writing to file... n = %d\n", n);
        
		// write into the file
		fwrite(&buf.data, sizeof(char), n, fp);
        // Send ACK
		
	}

	printf("File received successfully!\n"); // error check
	close (sock);
	fclose (fp);

	return 0;
}



// my_receive() function ensures the received data chunks are not corrupted
int my_receive (PACKET *recv_pck)
{
    int cs_in;
	int cs_calc;
	int	d;
	int r;
	int nbytes;

	HEADER			ack_packet;
	struct sockaddr	ret_addr;
	socklen_t		addr_size = sizeof (ret_addr);


	while (1)
	{
        // ssize_t recvfrom(int socket, void *restrict buffer, size_t length,
        //          int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
        // buffer: Points to the buffer where the message should be stored.
        // address: A null pointer, or points to a sockaddr structure in which the sending address is to be stored.
        // The length and format of the address depend on the address family of the socket.
        // address_len: Specifies the length of the sockaddr structure pointed to by the address argument.
		if ((nbytes = recvfrom (sock, recv_pck, sizeof(PACKET), 0, &ret_addr, &addr_size)) < 0)
			return -1;
	
        printf ("Received a UDP packet!\n");
        
		cs_in = recv_pck->header.checksum;
		recv_pck->header.checksum = 0;
        
        // recalculate checksum
		cs_calc = calc_checksum((char*)recv_pck, sizeof(HEADER) + recv_pck->header.len);

        // check if checksum matches, and the sequence number is correct too
        if (cs_in == cs_calc  &&  recv_pck->header.seq_ack == state)
		{
            printf ("Checksum passed! Sequence number matched!\n");

			// good packet
			printf("good packet\n");
			// create header of ack packet
            ack_packet.seq_ack = state; // seq #
            ack_packet.len = 0; // length = 0
            ack_packet.checksum = calc_checksum((char *)&ack_packet, sizeof(HEADER)); // addin checksum

            
            // simulating erroneous channel...corruption and loss
            // STUDENT WORK
			// lose rate of 10%
            
			if (rand() % 100 < 10) {
               printf("Simulating packet loss...\n");
               continue; // Skip processing this packet
            }
			
			// now we are expecting the next packet
			sendto(sock, &ack_packet, sizeof(HEADER), 0, (struct sockaddr *)&ret_addr, addr_size); // send
			state = 1 - state; // new state seq #
            
			return recv_pck->header.len;
		}
		else
		{
            printf ("Checksum/sequence number check failed!\n");

			// bad packet
            // create header of previous ack packet
			ack_packet.seq_ack = 1 - state; // send previous ack seq#
            ack_packet.len = 0; 
            ack_packet.checksum = calc_checksum((char *)&ack_packet, sizeof(HEADER));

            printf ("Resending ack for sequence number: %d...\n", ack_packet.seq_ack );

            
            // simulating erroneous channel...corruption and loss
            if (rand() % 100 < 10) {
               printf("Simulating packet loss...\n");
               continue; // Skip processing this packet
            }
			sendto(sock, &ack_packet, sizeof(HEADER), 0, &ret_addr, addr_size);
		}
	}
	return -1;
}



int calc_checksum(char *buf, int tbytes)
{
    int     i;
    char    cs = 0;
    char    *p;

	p = buf; 		//initialize p to the address of buf

	for(i = 0; i < tbytes; i++){
		cs ^= *p;   //xor each byte
		p++; 		// increment the pointer to next byte
	}

    return (int)cs;
}

