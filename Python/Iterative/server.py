import socket
import threading

HEADER = 64 #lenght of the header sent from the client that tells us the lenght of the rest of the message (bytes)

PORT = 5050 #set port number

SERVER = socket.gethostbyname(socket.gethostname()) #get the server host by name

FORMAT = 'utf-8' #form of decoding the bytes sent in the HEADER (string utf-8)

DISCONNECT_MESSAGE = "Disconnect"

#verify the host IIP adress
print(SERVER)

#structure of socket
ADDR = (SERVER,PORT)

#creating a socket (family (IPV4), type)
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

#binding ADDR to the socket created
server.bind(ADDR)

#conn allows us to communicate back to the established connection, 
#and addr is were the details about the connection are stored

def HandleClient(conn, addr): #will handle new connections
    print(f"[NEW CONNECTION] {conn, addr} connected")


    connected = True
    while connected:
        msg_lenght = conn.recv(HEADER).decode(FORMAT) #waits until it receives number of bytes specified from client over the socket
        if msg_lenght:
            msg_lenght = int(msg_lenght) #convert msg_lenght to int
            msg = conn.recv(msg_lenght).decode(FORMAT) #read the rest of the messsage
            if msg == DISCONNECT_MESSAGE:
                connected = False
                conn.send("Server says bye!".encode(FORMAT))
            print(f"[{addr}] {msg}")
            conn.send("Msg received".encode(FORMAT)) #send a receive message to client

    conn.close()


def start():
    server.listen()
    print(f"[LISTENING] server listening on {SERVER}")
    while True:
        conn, addr = server.accept() #waits for a new connection to the server
        thread = threading.Thread(target=HandleClient, args=(conn,addr)) #create a thread to handle the client
        thread.start()
        print(f"[ACTIVE CONNECTIONS] {threading.active_count()-1}")

print("[STARTING] server is starting...")
start()