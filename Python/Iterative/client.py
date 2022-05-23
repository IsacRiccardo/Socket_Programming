import socket

HEADER = 64 #lenght of the header sent from the client that tells us the lenght of the rest of the message (bytes)

PORT = 5050 #set port number

SERVER = "192.168.56.1" #server IP

FORMAT = 'utf-8' #form of decoding the bytes sent in the HEADER (string utf-8)

DISCONNECT_MESSAGE = 'Disconnect'

ADDR = (SERVER,PORT)

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)

def send(msg):
    message = msg.encode(FORMAT) #encode into a bytes like object in order to send the message
    msg_lenght = len(message)
    send_lenght = str(msg_lenght).encode(FORMAT) #the lenght of the message
    send_lenght += b' ' * (HEADER - len(send_lenght)) #must add spaces until the lenght is 64 bytes
    client.send(send_lenght)
    client.send(message)
    print(client.recv(2048).decode(FORMAT)) #print the lenght of the message received from the server

send("Hello world!")
input()
send("Disconnect")
