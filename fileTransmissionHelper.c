/* fileTransmissionHelper.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#define BUF 1024
#define PORT 6543
#define FILE_TO_SEND    "/home/schoko/Downloads/srv/"
void clrBuf(char *buffer)
{
    memset(buffer,'\0',BUF);
}
//Stringuebertragung mit vorangehender Laengenuebertragun per integer
int sendString(char *buffer,int socket){
    int commandsize=strlen(buffer)+1;
    if(send(socket,&commandsize,sizeof commandsize,0)==-1){
        perror("Error occured at sendString/Length");
        return -1;
    }
    if(send(socket, buffer, commandsize,0)==-1){
        perror("Error occured at sendString/String");
        return -1;
    }
    return 0;
}
int recvString(char *buffer,int socket){
    int commandsize;
    if(recv(socket,&commandsize,sizeof commandsize,0)!=sizeof commandsize){
        perror("Received bytes does not match with sizeof commandsize");
        return -1;
    }
    if(recv(socket, buffer, commandsize, 0)!=commandsize){
        perror("Received bytes does not match with commandsize");
        return -1;
    }
    return 1;
}
int castPortToLong(char * portarg)
{
    //Port wird in ein long gecastet und sollte ein ungueltiger Wert dafuer angegeben worden sein bricht das Programm ab.
    char * pEnd;
    long port=strtol(portarg,&pEnd,10);
    if(port<1024 || port>65536){
        perror("Invalid port. Port has to be in range of 1024-65536");
        return -1;
    }
    if(*pEnd!='\0'){
        perror("Not a valid port");
         return -1;
    }
    return port;
}
int sendFile(int socket, char *file_name)
{
//    char buffer[BUF];
    FILE *file_to_transfer = fopen(file_name, "r");
//    clrBuf(buffer);
    int bytes;
    if (file_to_transfer != NULL)
    {
        fseek (file_to_transfer , 0, SEEK_END);
        long bytes_to_transfer = ftell (file_to_transfer);
        rewind (file_to_transfer);
        printf("bytes_to_transfer: %lu\n", bytes_to_transfer);
        printf("sending\n");
        if (send(socket, &bytes_to_transfer, sizeof bytes_to_transfer, 0) == -1)
        {
            printf("Failed to send filesize\n");
            return -1;
        }

//        clrBuf(buffer);


//        if (strcmp(buffer, "-1") == 0)
//        {
//            return -1;
//        }
//        clrBuf(buffer);


//        if (strcmp(buffer, "-1") == 0)
//        {
//            return -1;
//        }
//        clrBuf(buffer);


        char * fileBuffer = (char*) malloc (sizeof(char)*bytes_to_transfer);
        fread (fileBuffer,1,bytes_to_transfer,file_to_transfer);

        long bytesleft = bytes_to_transfer;
        long bytessend = 0;
        long temp;
        printf("beforeWhile: %lu\n", bytesleft);
        while (bytesleft > 0)
        {
            printf("sending:%s, %li\n,",fileBuffer+bytessend,bytessend);
            if (send(socket, fileBuffer+bytessend, bytesleft, 0) == -1)
            {
                printf("Failed to send file\n");
                return -1;
            }
            printf("receiving:%lu\n,",sizeof temp);
            if (recv(socket, &temp,sizeof temp, 0) == -1)
            {
                printf("Failed to recv BytesLeft\n");
                return -1;
            }
            printf("receiving:%lu\n,", temp);
            bytessend += temp;
            bytesleft -= temp;

//            clrBuf(buffer);
        }

//        clrBuf(buffer);
        free(fileBuffer);
        fclose(file_to_transfer);


        return 0;
    }
    else
    {
        printf("Failed to open file\n");
        return -1;
    }
}

int recvFile(int socket, char *file_name, char *file_path)
{
//    char buffer[BUF];
    char filename[BUF];
    char fullPath[2*BUF];
    char *ptr;
    long check = 0;
    int loopCounter = 0;
    long bytes_to_receive;
    strcat(file_path, file_name);
    printf("%s \n",file_path);
//    clrBuf(buffer);
    printf("expecting: %lu\n",sizeof bytes_to_receive);
    if (recv(socket, &bytes_to_receive, sizeof bytes_to_receive, 0) == -1)
    {
        printf("Failed to receive Bytes\n");
        return -1;
    }
    printf("bytes_to_receive: %lu\n", bytes_to_receive);
//    clrBuf(buffer);
//    strcpy(buffer, "0");
//    printf("buffer: %s\n", buffer);
//    clrBuf(buffer);




    char *fileBuffer = malloc(sizeof(char)*bytes_to_receive);

    FILE *file_to_transfer = fopen(file_path, "w");
    printf("yee\n");
    if (file_to_transfer != NULL)
    {
        long bytesleft = bytes_to_receive;
        long bytesrecv = 0;
        long temp;
        printf("beforeWhile: %lu\n", bytesleft);
        while (bytesleft > 0)
        {
            printf("receiving:%s\nbytesleft:%li\n,",fileBuffer+bytesleft,bytesleft);
            temp = recv(socket, fileBuffer+bytesrecv, bytesleft, 0);
            printf("received: %lu\n", temp);
            if (temp == -1)
            {
                printf("Failed to recv file\n");
                return -1;
            }
            bytesrecv += temp;
            bytesleft -= temp;
//            sprintf(buffer, "%ld", temp);

            printf("sending:%lu\n,",temp);
            if (send(socket, &temp, sizeof temp, 0) == -1)
            {
                printf("Failed to bytes\n");
                return -1;
            }
//            clrBuf(buffer);
        }
        printf("looping done\n");
        printf("FileBuffer:%s\n,",fileBuffer);
//        clrBuf(buffer);
        fwrite (fileBuffer , 1, bytes_to_receive, file_to_transfer);
        free(fileBuffer);
        fclose(file_to_transfer);

//        clrBuf(buffer);

        return 0;

    }
    else
    {
        printf("Failed to open file\n");
        return -1;
    }
}
