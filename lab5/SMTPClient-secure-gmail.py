
# Behnam Dezfouli
#  CSEN, Santa Clara University

# This program implements a simple smtp mail client to send an email to a local smtp server
# the program runs the ping command, and emails the result using smtp.gmail.com

# NOTE: Do not forget to allow login from less secure apps in your gmail account. Otherwise gmail will complain about username and password.


import smtplib, ssl
import subprocess

# The program asks the user for her/his email address, password, and receiver’s email address 
port = 465  # For SSL
email_address = raw_input("Please enter your email address: ") # ask for your email address
password = raw_input("Please enter your password: ") # ask for your password
receiver_email = raw_input("Please enter receiver's email address: ") # ask for receiver email address


# The program connects to gmail SMTP server 

# ping google.com and save the result
# STUDENT WORK
result = subprocess.Popen(['ping', '-c', '2', 'google.com'], stdout=subprocess.PIPE)
ping_output = result.stdout.read()


print '\nNow contacting the mail server...'
# STUDENT WORK
context = ssl.create_default_context()




print '\nSending email...'


# STUDENT WORK
with smtplib.SMTP_SSL("smtp.gmail.com", port, context=context) as server:
    server.login(email_address, password)

    message = "Subject: Server Ping Result!\n\n" + ping_output
    server.sendmail(email_address, receiver_email, message)

