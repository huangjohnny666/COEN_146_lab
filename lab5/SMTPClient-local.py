# Behnam Dezfouli
#  CSEN, Santa Clara University

# This program implements a simple smtp mail client to send an email to a local smtp server

# Note:
# Run a local smtp mail server using the following command before running this code:
# python -m smtpd -c DebuggingServer -n localhost:6000


from socket import *
import ssl


# Choose a mail server
mailserver = 'localhost'


# Create socket called clientSocket and establish a TCP connection with mailserver
# STUDENT WORK
    
# Creating socket
clientSocket = socket(AF_INET, SOCK_STREAM)
    
# Setting socket address to SO_REUSEADDR
clientSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

# Port number may change according to the mail server
# STUDENT WORK
# Connecting socket
clientSocket.connect((mailserver, 6000))

#service ready message
recv = clientSocket.recv(1024).decode()
print(recv)
if recv[:3] != '220':
    print('220 reply not received from server.')


# Send HELO command along with the server address
# STUDENT WORK
# Create domain for self then send to socket
sender_domain = 'myhost'
message = 'HELO ' + sender_domain  + '\r\n'  
clientSocket.send(message.encode())


# OK message
server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '250':
    print('250 reply not received from server.')


# Send MAIL FROM command and print server response
# STUDENT WORK
reverse_path = 'this@gmail.com'
message = 'MAIL FROM:' + reverse_path + '\r\n'
clientSocket.send(message.encode())


#OK message
server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '250':
    print('250 reply not received from server.')


# Send RCPT TO command and print server response
# STUDENT WORK
forward_path = 'that@gmail.com'
message = 'RCPT TO:' + forward_path + '\r\n'
clientSocket.send(message.encode())


# OK message
server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '250':
    print('250 reply not received from server.')


# Send DATA command and print server response
# STUDENT WORK
clientSocket.send("DATA\r\n".encode())

server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '354':
    print('354 reply not received from server.')


# Send message data.
# STUDENT WORK
data = 'SUBJECT: Greetings To you!\r\n'
data += 'This is line 1 \r\n'
data += 'This is line 2 \r\n'


# Message ends with a single period
# STUDENT WORK
data += '.\r\n'
clientSocket.send(data.encode())

# Wait for server response
server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '250':
    print('250 reply not received from server.')


# Send QUIT command and get server response
# STUDENT WORK
quit_command = 'QUIT\r\n'
clientSocket.send(quit_command.encode())

# Wait for server response 
server_response = clientSocket.recv(1024).decode()
print(server_response)
if server_response[:3] != '221':
    print('221 reply not received from server.')


# Close socket once done
clientSocket.close()
