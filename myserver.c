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
    char file_path[BUF];
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
    if(port==-1)
    {
        return EXIT_FAILURE;
    }
    address.sin_port = htons (port);

    if (bind ( create_socket, (struct sockaddr *) &address, sizeof (address)) != 0)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }
    listen (create_socket, 5);

    addrlen = sizeof (struct sockaddr_in);
    printf("Waiting for connections...\n");
    while (1)
    {
        
        new_socket = accept ( create_socket, (struct sockaddr *) &cliaddress, &addrlen );
        pid_t pid = fork();

        if (pid == 0)
        {
            printf("Child process created\n");
        }
        else if (pid > 0)
        {
            printf("Child process PID:%i \n",pid);
            close(new_socket);
            continue;
        }
        else
        {
            perror("Fatal error while forking. Exiting\n");
            return EXIT_FAILURE;
        }
        if (new_socket > 0)
        {
            printf ("Client connected from %s:%d...\n", inet_ntoa (cliaddress.sin_addr),ntohs(cliaddress.sin_port));
            strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
            sendString(buffer,new_socket);
        }
        do
        {

            clrBuf(buffer);
            size = recvString(buffer,new_socket);
            if( size > 0)
            {
//                buffer[size] = '\0';
                printf ("Message received: %s\n", buffer);
                //LIST
                if (strncmp(buffer, "list",4) == 0)
                {
                    clrBuf(listbuffer);
                    DIR *dir;
                    struct dirent *ent;
                    //Ausgabe der Dateien im Ordner
                    if ((dir = opendir (argv[1])) != NULL)
                    {
//                        clrBuf(buffer);
                        while ((ent = readdir (dir)) != NULL)
                        {
                            if(ent->d_type==DT_REG){
                                strcat(listbuffer,ent->d_name);
                                strcat(listbuffer,"\n");
                            }
                        }
                        closedir (dir);
                    }
                    else
                    {
                        perror ("Error while opening the Folder\n");
                        return EXIT_FAILURE;
                    }
                    sendString(listbuffer,new_socket);
                }
                //GET
                else if(strncmp(buffer, "get",3)  == 0)
                {
                    memset(file_name,'\0',sizeof file_name);
                    strncpy(file_name,&buffer[4],strlen(buffer)-4);
                    printf("%lu \n",strlen(file_name));

                    file_name[strcspn(file_name, "\n")] = '\0';
                    strcpy(file_name_helper,file_name);
                    strcpy(file_name,argv[1]);
                    strcat(file_name,file_name_helper);
                    printf("Receiving file %s from client\n",file_name);
                    if(sendFile(new_socket,file_name)==0){
                        printf("File %s successfully received\n",file_name);
                    }
                }
                //PUT
                else if(strncmp(buffer, "put",3)  == 0)
                {   
                    memset(file_name,'\0',sizeof file_name);
                    strncpy(file_name,&buffer[4],strlen(buffer)-4);
                    printf("%lu \n",strlen(file_name));

                    file_name[strcspn(file_name, "\n")] = '\0';
                    strcpy(file_path,argv[1]);
                    //printf("PUT-Filename:%s \n",file_name);
                    //printf("PUT-Filepath:%s \n",file_path);
                    printf("Sending file %s to client\n",file_name);
                    if(recvFile(new_socket,file_name,file_path)==0){
                        printf("File %s successfully sent\n",file_name);
                    }
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
