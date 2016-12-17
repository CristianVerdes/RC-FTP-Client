#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>


using namespace std;

#define LENGTH 512
/* codul de eroare returnat de anumite apeluri */

/* portul de conectare la server*/
int port;

string encryptDecrypt(string toEncrypt) {
    char key = 'K'; //Any char will work
    string output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key;

    return output;
}

int upload(int sd, char* f_name){
    string conditie = "continue";
    char sdbuf[LENGTH]; // Send buffer
    printf("[client] send %s to the client...", f_name);
    FILE *fp = fopen(f_name, "r");
    if(fp == NULL)
    {
        printf("ERROR: File %s not found.\n", f_name);
        exit(1);
    }
    bzero(sdbuf, LENGTH);
    int f_block_sz;
    while((f_block_sz = fread(sdbuf, sizeof(char), LENGTH, fp))>0)
    {
        if(write(sd, sdbuf, f_block_sz) < 0)
        {
            printf("ERROR: Failed to send file %s.\n", f_name);
            break;
        }
        bzero(sdbuf, LENGTH);
        if (write(sd, (void *) conditie.c_str(), sizeof(conditie)) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
    }
    //STOP
    conditie = "stop";
    if (write(sd, (void *) conditie.c_str(), sizeof(conditie)) <= 0) {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }
    if (write(sd, (void *) conditie.c_str(), sizeof(conditie)) <= 0) {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }
    printf("ok! Fisier Transferat...\n");
    return 0;
}

int main (int argc, char *argv[])
{
    int sd;			// descriptorul de socket
    char msg[100];
    char msg2[20000];
    struct sockaddr_in server;	// structura folosita pentru conectare
    int fd;
    struct stat file_stat;
    char file_size[256];
    ssize_t len;
    int index = 0;


    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    //START THE FUN!!!
    char conditie[100];
    while(1){
        index = 0;
        //Read from server
        if (read (sd, msg2, sizeof(msg2)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        //Showing what we received
        printf ("%s\n", msg2);

        //Read what the client is typing
        bzero((void *) msg, 100);
        read(0, (void *) msg, sizeof(msg));
        printf("[client] Am citit %s", msg);

        //Upload
        if(strcmp(msg,"upload\n")==0) {
            index = 1;
            //Trimitem la server comanda upload
            strncpy(conditie, msg, strlen(msg) - 1);
            if (write(sd, (void *) conditie, sizeof(conditie)) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            //Citim de la server
            if (read(sd, msg2, sizeof(msg2)) < 0) {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("%s\n", msg2);

            //Citim de la client numele fisierului
            bzero((void *) msg, 100);
            read(0, (void *) msg, sizeof(msg));
            printf("[client] Am citit %s", msg);

            //Trimitem la server numele fisierului
            bzero(conditie, 100);
            strncpy(conditie, msg, strlen(msg) - 1);
            if (write(sd, (void *) conditie, sizeof(conditie)) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("\n bal bal bal \n");
            upload(sd, conditie);
        }

        //Ending condition
        if(strcmp(msg,"quit\n")==0){
            strncpy(conditie,msg, strlen(msg)-1);
            if (write(sd, (void *) conditie, sizeof(conditie)) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("Exit...");
            return 0;
        }
        else {
            //Sending back the message
            if(strcmp(msg2,"[server]Introduceti o parola: ")==0){
                strncpy(conditie,msg, strlen(msg)-1);
                string s = encryptDecrypt(string(conditie));
                printf("Parola criptata: %s",s.c_str());
                if (write(sd, (void *) s.c_str(), sizeof(s)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }
                bzero(msg, 100);
                bzero(conditie, 100);
            }
            else{
                    strncpy(conditie, msg, strlen(msg) - 1);
                    if (write(sd, (void *) conditie, sizeof(conditie)) <= 0) {
                        perror("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    bzero(msg, 100);
                    bzero(conditie, 100);

            }
        }

        bzero(msg2, 2000);

    }
}