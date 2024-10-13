
# Behnam Dezfouli
# CSEN, Santa Clara University

# This program implements a simple web server that serves html and jpg files

# Input arguments:
# argv[1]: Sever's port number


from socket import *  # Import socket module
import sys            # To terminate the program
import os

if __name__ == "__main__":

    # check if port number is provided
    if len(sys.argv) != 2:
        print ('Usage: python %s <PortNumber>' % (sys.argv[0]))
        sys.exit()
    
    
    # STUDENT WORK
    # Creating socket
    socket_fd = socket(AF_INET, SOCK_STREAM)
    
    # Setting sock address to SO_REUSEADDR
    socket_fd.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

    # Binding socket
    port = int(sys.argv[1])
    socket_fd.bind(("127.0.0.1", port))

    # Socket listening
    socket_fd.listen(1)

    # Server should be up and running and listening to the incoming connections
    while True:
        print('The server is ready to respond to incoming requests...')


	# Accept incoming connection from client
        connection_fd, client_address = socket_fd.accept()

        print("Accepted request from client \n")

        try:
	        # receive data from client
            received_data = connection_fd.recv(1024)



            parsed_data = received_data.split("\n") # splits all received data

            requested_file = parsed_data[0] # takes second string "/index.html" or "/.jpg" 
            requested_file = requested_file.split(" ")[1].replace("/","") # and removes the slash from beginning
            
            # case with no file specified
            if not requested_file:
                requested_file = "index-1.html"

            root, file_extension = os.path.splitext(requested_file) # separate the extension file_path
            file_extension = file_extension.replace(".","")# remove the .

            if (file_extension == 'html'):
                response_headers = { 'Content-Type': 'text/html; encoding=utf8' } 
            elif (file_extension == 'jpg'):
                response_headers = { 'Content-Type': 'image/jpeg; encoding=utf8' }
            else:
                print ('Invalid file type, we only support html and jpg!')
                continue

            with open(requested_file, 'rb') as file: # open file for reading in binary
                content = file.read()
            
            # Printing into socket, createing the header line of http response
            header_line = "HTTP/1.1 200 OK\r\n"
            header_line += "Content-Type: " + response_headers.get("Content-Type") + "\nContent-Length: " + str(len(content))
            response=""
            response+=header_line+'\n\n'
            response+=content # connect the response with content read

            connection_fd.send(response)   # sends content to the socket and encodes to bytes
        
            connection_fd.close() # close the connection

        #Raised when an I/O operation (such as a print statement, the built-in open() function or a method of a file object) fails for an I/O-related reason, e.g., "file not found" or "disk full"
        except IOError:

		# STUDENT WORK
            error_line = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; encoding=utf8\r\nContent-Length: 13\r\n\r\n404 Not Found!" # create header line for 404 error and send
            connection_fd.send(error_line)
            connection_fd.close()
       
    socket_fd.close() # close socket
