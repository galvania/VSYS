/* myclient.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "fileTransmissionHelper.h"

#define FILE_TO_SEND "/home/schoko/Downloads/srv/"
#define BUF 1024

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
int main (int argc, char **argv)
{
    int create_socket;
    char buffer[BUF];
    struct sockaddr_in address;
    int size;
    char file_name[BUF];
    char file_path[BUF];
    char file_name_helper[BUF];
    char file_size[256];
    long port;
    if( argc < 3 )
    {
        printf("Usage: %s Address Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    port=castPortToLong(argv[2]);

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (port);
    inet_aton (argv[1], &address.sin_addr);

    if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)
    {
        printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
        // size=recvString(buffer,create_socket);
        // if(size==-1){
        //     return EXIT_FAILURE;
        // }
        size=recv(create_socket,buffer,BUF-1, 0);
        if (size>0)
        {
            buffer[size]= '\0';
            printf("%s",buffer);
        }
    }
    else
    {
        perror("Connect error - no server available");
        return EXIT_FAILURE;
    }

    do
    {
        int commandsize=strlen(buffer);
        clrBuf(buffer);
        printf ("Send message: ");
        fgets (buffer, BUF, stdin);
        // send(create_socket, &commandsize, sizeof commandsize, 0);

        // send(create_socket, buffer, strlen (buffer), 0);
        sendString(buffer,create_socket);
        if((strcmp (buffer, "list\n") == 0))
        {
            clrBuf(buffer);
            recv(create_socket,&commandsize,sizeof commandsize,0);

            size = recv (create_socket, buffer, commandsize, 0);
//            size=recv(create_socket,buffer,BUF-1, 0);
            printf ("Files: \n%s", buffer);
        }
        else if (startsWith("get",buffer))
        {
            memset(file_name,'\0',sizeof file_name);
            strncpy(file_name,&buffer[4],strlen(buffer)-4);
            printf("fnf:%s \n",file_name);

            strcpy(file_path,argv[2]);
            file_name[strcspn(file_name, "\n")] = '\0';
            printf("fna:%s \n",file_path);
            recvFile(create_socket,file_name, file_path);
        }
        else if (startsWith("put",buffer))
        {

            memset(file_name,'\0',sizeof file_name);
            strncpy(file_name,&buffer[4],strlen(buffer)-4);
            printf("%s \n",file_name);

            file_name[strcspn(file_name, "\n")] = '\0';
            strcpy(file_name_helper,file_name);
            strcpy(file_name,argv[2]);
            strcat(file_name,file_name_helper);
            printf("%s \n",file_name);
            sendFile(create_socket,file_name);
        }
    }
    while (strncmp(buffer, "quit",4) != 0);
    close (create_socket);
    return EXIT_SUCCESS;
}
