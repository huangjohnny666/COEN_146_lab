// SCU COEN 146
//
// This program implements the client side of stop & wait protocol
// the client transfers a file to the server
//
//
// For the stop and wait protocolL, the assumptions are:
//      -- packet corruption, and packet loss


#include	"tfv1.h"

// global variables
int state = 0; // only two states: 0 and 1
int sock;
struct sockaddr_in serverAddr;

socklen_t addr_size;



// list of functions
int main (int, char *[]);

// to reliably send a chunk of data
void my_send (char *, int);

// part of the my_send() function
void recv_ack (PACKET *, int);

// computes checksum using XOR of bytes
int calc_checksum(char *, int);




int main (int argc, char *argv[])
{
    char	buff[SIZE];
    FILE	*fp; // file to be sent to the server
    int		r;
    
    if (argc != 5)
    {
        printf ("usage:usage: ./client port IP source_file dest_file\n");
        return 1;
    }
    
    
    // configure address
    // STUDENT WORK
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, argv[2], &serverAddr.sin_addr);
    
    
    // create UDP socket
    if ((sock = socket (PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf ("socket error\n");
        return 1;
    }
    printf ("UDP socket has been created!\n");
    
    if ((fp = fopen (argv[3], "rb")) == NULL)
    {
        printf ("fopen error\n");
        return 1;
    }
    printf ("Source file opened successfully!\n");

    
    printf ("Sending file name... \n");
    // STUDENT WORK - send file + length of file
    my_send(argv[3], strlen(argv[3])); // question
    printf ("Done!\n");

    
    printf ("Now sending source file contents... \n");
    // STUDENT WORK - loop through file and send data through my_send
    while((r=fread(buff, 1, SIZE, fp)) > 0)
    {
	my_send(buff, r);
    }	
    printf ("Done!\n");

    
    
    // After transmitting the file, a packet with no data (len = 0) is sent to
    // notify the receiver that the file transfer has completed
    printf ("Informing the server about the completion of file transmission...\n");
    // STUDENT WORK
    // sends empty packet with length 0
    my_send("",0);
    printf ("Done!\n");
    
    
    fclose (fp);
    close (sock);
    
    return 0;
    
}


//***********************
// sends data (of size nbytes) reliably to the sender
// **********************
void my_send (char *data, int nbytes)
{
    PACKET	buf;
    int		r;
    
    // STUDENT WORK
    // Initialize all values in header and copy data into buf
    buf.header.seq_ack = state;
    buf.header.len = nbytes;
    buf.header.checksum = 0;
    strcpy(buf.data, data);

    
    
    // simulating erroneous channel... corruption and loss
    // the probability of correct checksum is 80%
    r = rand () % 10;
    if (r < 8)
        buf.header.checksum = calc_checksum((char*)&buf, sizeof(HEADER) + nbytes);
    else
        printf ("Packet got corrupted on the way!\n");
    
    
    // the probability of no packet loss is 80%
    // STUDENT WORK
    r = rand () % 10;
    if (r < 8)
    //sending packet via conectionless UDP
	sendto(sock, &buf, sizeof(HEADER) + nbytes, 0, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    else
        printf ("Packet was lost in transmission!\n");
    
    
    // wait for ack and change value of state
    // STUDENT WORK
    recv_ack(&buf, sizeof(buf));
    state = !state;
    
    return;
}



//***********************
// wait for ack
// **********************
void recv_ack (PACKET *buf, int nbytes)
{
    HEADER	receive_buf;
    int		cs;
    int		r;
    struct timeval tv;
    int		rv;
    int		counter = 0;
    
    // The fd_set data type represents file descriptor sets for the select function
    // It is actually a bit array
    // (set of socket descriptors)
    fd_set readfds;
    
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    while (1)
    {
        while (1)
        {
            FD_ZERO (&readfds); // Clear an fd_set
            
            // void FD_SET (int filedes, fd_set *set)
            // This macro adds filedes to the file descriptor set set.
            FD_SET (sock, &readfds);
            
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            
            
            // int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
            // maxfd: maximum number of descriptor ready.
            // timeout: How long to wait for select to return.
            //The select function blocks , till an activity occurs
            
            // On success, select() and pselect() return the number of file
            // descriptors contained in the three returned descriptor sets (that is,
            // the total number of bits that are set in readfds, writefds,
            // exceptfds) which may be zero if the timeout expires before anything
            // interesting happens
            
            // in the following we only pass readfds because we are interested in reading only
            rv = select (sock + 1, &readfds, NULL, NULL, &tv);
            
            
            // return value is 1 when there is a bit set in readfds
            // so there is data to be read
            if (rv == 1)
            {
                // STUDENT WORK
		// break from while loop
		        break;		
            }
            
            // this means select() returned due to timeout event
            else if (rv == 0)
            {
		//increment counter and check if == 3 
                printf ("timeout\n");
                counter++;
                if (counter == 3)
                {
                    //STUDENT WORK
                    //Retransmit packet after 3 timeouts and reset counter
                    printf("Retransmitting packet... \n");
                    my_send(buf->data, nbytes);
                    counter = 0; 
                    return;
                }
            }
        }
        
        
        
        // we break the previous while(1) loop when there is data to be read (ack received)
        printf("Going to receive packet\n");
        recvfrom (sock, &receive_buf, sizeof(HEADER), 0, NULL, NULL);
        
        cs = receive_buf.checksum;
        receive_buf.checksum = 0;
        
        
        // recalculate checksum of the received ack packet
       	// STUDENT WORK
        int cs_calc = calc_checksum((char *)&receive_buf, sizeof(receive_buf));
        
        
        // a bad ack has been received 
	// if checksum is different from calculated check sum
	// or if seq number of received pkt is not = to current state
        // STUDENT WORK
        if (cs_calc != cs || receive_buf.seq_ack != state)
        {
	        printf("Incorrect ack received... resending packet \n");
            my_send(buf->data, nbytes);
        }
        // resend packet
        // simulating erroneous channel...corruption and loss
        // STUDENT WORK
	// the probability of correct checksum is 80%
	r = rand () % 10;
	if (r < 8)
	    buf->header.checksum = calc_checksum((char *)&buf, sizeof(HEADER) + nbytes);
	else
	    printf ("Packet got corrupted on the way!\n");
    
    
	// the probability of no packet loss is 80%
	// STUDENT WORK
	r = rand () % 10;
	if (r < 8)
	//sending packet via conectionless UDP
	    sendto(sock, &buf, sizeof(HEADER) + nbytes, 0, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	else
	    printf("Packet was lost in transmission!\n");
    

    }
}




//***********************
// calculate checksum by using XOR
// **********************
int calc_checksum(char *buf, int tbytes)
{
    int		i;
    char	cs = 0;
    char 	*p;
   
    p = buf; // initialize p to the address of buf
    // STUDENT WORK
    for(i = 0; i<tbytes; i++)
    {
	cs ^= *p; // xor each byte
	p++; // increment the pointer to next byte
    }
    

    return (int)cs;
}





