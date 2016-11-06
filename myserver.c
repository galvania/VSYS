/* myserver.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "fileTransmissionHelper.h"
#define BUF 1024
#define PORT 6543
#define FILE_TO_SEND    "/home/schoko/Downloads/srv/"

int main (int argc, char **argv)
{
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    char listbuffer[BUF];
    int size;
    struct sockaddr_in address, cliaddress;

    char file_name[BUF];
    char file_name_helper[BUF];
    char file_size[256];
    int offset;
    int remain_data;
    char file_path[BUF];
    int commandsize;
    long port;
    char * pEnd;
    create_socket = socket (AF_INET, SOCK_STREAM, 0);
    if( argc < 2 )
    {
        printf("Usage: %s Directory Port\n ", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    port=castPortToLong(argv[2]);
    if(port==-1){
        return EXIT_FAILURE;
    }
    printf("%lu",port);
    address.sin_port = htons (port);

    if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }
    listen (create_socket, 5);

    addrlen = sizeof (struct sockaddr_in);

    while (1)
    {
        printf("Waiting for connections...\n");
        new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen );
        if (new_socket > 0)
        {
            printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
            strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
            commandsize=strlen(buffer);
            //send(new_socket,&commandsize,sizeof commandsize,0);
            send(new_socket, buffer, strlen(buffer),0);
            //sendString(buffer,new_socket);
        }
        do
        {

            clrBuf(buffer);
            recv(new_socket,&commandsize,sizeof commandsize,0);

            size = recv (new_socket, buffer, commandsize, 0);
            if( size > 0)
            {
//                buffer[size] = '\0';
                printf ("Message received: %s\n", buffer);
                if (strncmp(buffer, "list",4) == 0)
                {
                    DIR *dir;
                    struct dirent *ent;
                    printf("DIR %s T \n",argv[0]);
                    if ((dir = opendir ("./")) != NULL)
                    {
//                        clrBuf(buffer);
                        while ((ent = readdir (dir)) != NULL)
                        {
                            strcat(listbuffer,ent->d_name);
                            strcat(listbuffer,"\n");
                        }
                        closedir (dir);
                    }
                    else
                    {
                        perror ("Fehler beim oeffnen des Ordners");
                        return EXIT_FAILURE;
                    }
                    commandsize=strlen(listbuffer);
                    send(new_socket, &commandsize, sizeof commandsize, 0);

                    send(new_socket, listbuffer, commandsize, 0);

                }
                else if(strncmp(buffer, "get",3)  == 0)
                {
                    memset(file_name,'\0',sizeof file_name);
                    strncpy(file_name,&buffer[4],strlen(buffer)-4);
                    printf("%lu \n",strlen(file_name));

                    file_name[strcspn(file_name, "\n")] = '\0';
                    strcpy(file_name_helper,file_name);
//                    clrBuf(file_name);

                    strcpy(file_name,argv[1]);
                    strcat(file_name,file_name_helper);
                    printf("%s \n",file_name);
                    sendFile(new_socket,file_name);
                }
                else if(strncmp(buffer, "put",3)  == 0)
                {
                    memset(file_name,'\0',sizeof file_name);
                    strncpy(file_name,&buffer[4],strlen(buffer)-4);
                    printf("%lu \n",strlen(file_name));

                    file_name[strcspn(file_name, "\n")] = '\0';
                    strcpy(file_path,argv[1]);
                    printf("PUT-Filename:%s \n",file_name);
                    printf("PUT-Filepath:%s \n",file_path);
                    recvFile(new_socket,file_name,file_path);
                }
//                clrBuf(buffer);
            }
            else if (size == 0)
            {
                printf("Client closed remote socket\n");
                break;
            }
            else
            {
                perror("recv error");
                return EXIT_FAILURE;
            }
        }
        while (strncmp (buffer, "quit", 4)  != 0);
        close (new_socket);
    }
    close (create_socket);
    return EXIT_SUCCESS;
}
