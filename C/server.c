#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080
#define BUFFLEN 1000
#define NUM_THREADS 5

#define WARNING "No .html file requested!"
#define HTTPHEADER "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\nConnection: close\nCache-Control: no-store\nContent-Length: "

struct argumente{
    char *header;
    int socket;
    long id;
};

//variabile globale
int server_fd;
long valread, t;
struct sockaddr_in address;
int addrlen = sizeof(address);

//Continutul fisierului html
char HtmlText[BUFFLEN] = {0};

pthread_t thread[NUM_THREADS];

void ResetBuffer(char*);

void InitHeader(char *Header)
{
    strcpy(Header,HTTPHEADER);
}

//Functie folosita pentru a contrui metoda de POST
//header - header pentru POST
//htmltext - continut fisier html
//lenght - lungime fisier html
char *ConstructPostMethod(char *header,char *htmltext, int lenght)
{
    char string[4];

    sprintf(string,"%d",lenght);
    strcat(header,string);
    //printf("%s\n",header);
    strcat(header,"\n\n");
    //printf("%s\n",header);
    strcat(header,htmltext);
    //printf("%s\n",header);

    return header;
}

void ConstructHeader(FILE *html)
{
    //Buffer folosit pentru a citi fiecare linie a fisierului html
    char HtmlBuffer[BUFFLEN-500] = {0};

    while(fgets(HtmlBuffer,BUFFLEN-500,html))
    {
       strcat(HtmlText,HtmlBuffer);
    }
    ResetBuffer(HtmlBuffer);

    //printf("%s\n",HtmlText);
}

//Preluarea paginii HTML
char* BufferProcessing(char *buffer) 
{
    short int contor = 0;
    char *pointer = NULL;

    //impart headerul in cuvinte separate prin spatiu
    pointer = strtok(buffer," ");

    //preiau al doilea cuvant
    while(pointer != NULL && contor < 2)
    {
        contor++;
        if(pointer[0] == '/' && pointer[1] != '\0')
        {
            return (pointer + 1);
        }
        else{
            pointer = strtok(NULL," ");
        }
    }
    //returnez un mesaj de evertizare in cazul in care nu a fost ceruta nici o pagina html
    return WARNING;
    
}

//functie folosita pentru a reseta buffere
void ResetBuffer(char *string1)
{
    strcpy(string1,"");
}

void *ClientHandler(void *args)
{
    // buffer de citire //
    char buffer[BUFFLEN] = {0};

    char *HtmlFile = NULL;

    // cast //
    struct argumente *arg = (struct argumente *)args;

    //********************* Reading and processing get method ********************

    valread = read( arg->socket , buffer, BUFFLEN);

    if(valread == -1)
    {
        printf("[ERROR] Cannot read browser request!\n");
    }

    HtmlFile = BufferProcessing(buffer); 

    printf("%s\n",HtmlFile);
    
    //pointer in care se retine metoda de post
    char *PostMethod = NULL;

    //valdarea paginii html din request
    if(HtmlFile == NULL)
    {
        printf("[ERROR] Cannot process browser request!\n");
    }
    else 
    {
        if(!strcmp(HtmlFile,WARNING))
        {
            printf("[WARNING] %s\n",WARNING);
        }
        else
        {
            printf("[GET METHOD] file: %s\n",HtmlFile);
        }

        FILE *html = fopen(HtmlFile,"r");

        if(html == NULL)
        {
            html = fopen("404.html","r"); //preia pagina default de eroare
        }

        //******************** Constructing header and post method ***********************
           
        ConstructHeader(html);
            
        PostMethod = ConstructPostMethod(arg->header,HtmlText,strlen(HtmlText));

        //printf("%s\n",PostMethod);

        //*************************** Sending info to client ******************************

        if(send(arg->socket , PostMethod , strlen(PostMethod),0)==-1)
            printf("[ERROR] Cannot send html file to client\n");
        else
            printf("------------------ Html page sent -------------------\n");

        ResetBuffer(PostMethod);
        ResetBuffer(HtmlText);
    }
    close(arg->socket);
    printf("------------------ Client handled -------------------\n");

    printf("----------------- Thread Terminated -----------------\n");
    pthread_exit(NULL);
}

void main()
{
    int rc, new_socket;
    struct argumente argumente;

    //header-ul necesar pentru a afisa continutul fisierului html pe browser
    char Header[BUFFLEN] = {0};
    
    // Crearea socket-ului //

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurarea structurii address //

    address.sin_family = AF_INET;   //IPV4
    address.sin_addr.s_addr = htons(INADDR_ANY);   
    address.sin_port = htons( PORT ); //Atribuirea portului
    
    memset(address.sin_zero, '\0', sizeof(address.sin_zero)); //Umplerea sin_zero cu '\0'
    
    // Atribuirea structurii address socket-ului creat //

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("[ERROR] In bind");
        exit(EXIT_FAILURE);
    }

    // Setarea socket-ului pentru ascultare //

    if (listen(server_fd, 10) < 0) 
    {
        perror("[ERROR] In listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {

        printf("\n++++++++++++ Waiting for new connection +++++++++++++\n\n");

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("[ERROR] In accept");
            exit(EXIT_FAILURE);
        }

        if(t == NUM_THREADS)
            t = 0;
        
        // initializare header la fiecare conectiune noua //
        InitHeader(Header);

        // atribuire argumente pentru tratarea clientului //
        argumente.socket = new_socket;
        argumente.header = Header;
        argumente.id = t;

        //************************* Thread creation **************************

        printf("[MAIN] Creare fir de executie: %ld\n", ++t);
                
        rc = pthread_create(&thread[t], NULL, ClientHandler, (void *)&argumente);

        if (rc)
        {
            printf("Codul erorii este: %d\n", rc);
            exit(-1);
        }
            
    }
   pthread_exit(NULL);

}