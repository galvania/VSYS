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
void clrBuf(char *buffer)
{
    memset(buffer,'\0',BUF);
}
int sendInt(int value,int socket){
    int retvalue=htonl(value);
    if(send(socket,&retvalue,sizeof value,0)==-1){
        perror("Error during sendInt\n");
        return -1;
    }
    return 0;
}
int recvInt(int *value,int socket){
    int retvalue;
    if(recv(socket,&retvalue,sizeof retvalue,0)!=sizeof retvalue){
        perror("Error during recvInt");
        return -1;
    }
    *value=ntohl(retvalue);
    return 0;
}
int sendString(char *buffer,int socket){
    //Stringuebertragung mit vorangehender Laengenuebertragung per integer
    int commandsize=strlen(buffer)+1;
    if(sendInt(commandsize,socket)==-1){
        perror("Error occured at sendString/Length\n");
        return -1;
    }
    // if(send(socket,&commandsize,sizeof commandsize,0)==-1){
    //     perror("Error occured at sendString/Length");
    //     return -1;
    // }
    if(send(socket, buffer, commandsize,0)==-1){
        perror("Error occured at sendString/String\n");
        return -1;
    }
    return 0;
}
int recvString(char *buffer,int socket){
    //Stringuebertragung mit vorangehender Laengenuebertragung per integer
    //Gibt 1 zurueck bei Erfolg
    int commandsize;
    if(recvInt(&commandsize,socket)==-1){
        perror("Received bytes does not match with sizeof commandsize\n");
        return -1;
    }
    // if(recv(socket,&commandsize,sizeof commandsize,0)!=sizeof commandsize){
    //     perror("Received bytes does not match with sizeof commandsize");
    //     return -1;
    // }
    if(recv(socket, buffer, commandsize, 0)!=commandsize){
        perror("Received bytes does not match with commandsize\n");
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
        perror("Invalid port. Port has to be in range of 1024-65536\n");
        return -1;
    }
    if(*pEnd!='\0'){
        perror("Not a valid port\n");
         return -1;
    }
    return port;
}
int sendFile(int socket, char *file_name)
{
    int code=1;
    if( access( file_name, F_OK )== -1 ) {
        // file doesn't exists
        code=0;
        sendInt(code,socket);
        printf("File doesn't exist\n");
        return -1;
    }
    sendInt(code,socket);
    FILE *file_to_transfer = fopen(file_name, "r");
    //    clrBuf(buffer);
    int bytes;
    if (file_to_transfer != NULL)
    {
        fseek (file_to_transfer , 0, SEEK_END);
        long bytes_to_transfer = ftell (file_to_transfer);
        rewind (file_to_transfer);
        //printf("bytes_to_transfer: %lu\n", bytes_to_transfer);
        //printf("sending\n");
        if (send(socket, &bytes_to_transfer, sizeof bytes_to_transfer, 0) == -1)
        {
            printf("Failed to send filesize\n");
            fclose(file_to_transfer);
            return -1;
        }

        char * fileBuffer = (char*) malloc (sizeof(char)*bytes_to_transfer);
        fread (fileBuffer,1,bytes_to_transfer,file_to_transfer);

        long bytesleft = bytes_to_transfer;
        long bytessend = 0;
        long temp;
        //printf("beforeWhile: %lu\n", bytesleft);
        while (bytesleft > 0)
        {
            //printf("sending:%s, %li\n",fileBuffer+bytessend,bytessend);
            if (send(socket, fileBuffer+bytessend, bytesleft, 0) == -1)
            {
                printf("Failed to send file\n");
                free(fileBuffer);
                fclose(file_to_transfer);
                return -1;
            }
            //printf("receiving:%lu\n,",sizeof temp);
            if (recv(socket, &temp,sizeof temp, 0) == -1)
            {
                printf("Failed to recv BytesLeft\n");
                free(fileBuffer);
                fclose(file_to_transfer);
                return -1;
            }
            //printf("receiving:%lu\n,", temp);
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
        fclose(file_to_transfer);
        return -1;
    }
}

int recvFile(int socket, char *file_name, char *file_path)
{
    int fexits;   
    if(recvInt(&fexits,socket)==-1){
        return -1;
    }
    if(fexits!=1){
        printf("File does not exist\n");
        return -1;
    }
    char filename[BUF];
    char fullPath[2*BUF];
    char *ptr;
    long check = 0;
    int loopCounter = 0;
    long bytes_to_receive;
    strcat(file_path, file_name);
    printf("Sending File\n");
    //printf("%s \n",file_path);
    //printf("expecting: %lu\n",sizeof bytes_to_receive);
    //send filesize

    if (recv(socket, &bytes_to_receive, sizeof bytes_to_receive, 0) == -1)
    {
        printf("Failed to receive Bytes\n");
        return -1;
    }
    //printf("bytes_to_receive: %lu\n", bytes_to_receive);
    char *fileBuffer = malloc(sizeof(char)*bytes_to_receive);

    FILE *file_to_transfer = fopen(file_path, "w");
    if (file_to_transfer != NULL)
    {
        long bytesleft = bytes_to_receive;
        long bytesrecv = 0;
        long temp;
        //printf("beforeWhile: %lu\n", bytesleft);
        while (bytesleft > 0)
        {
            //printf("receiving:%s\nbytesleft:%li\n,",fileBuffer+bytesleft,bytesleft);
            temp = recv(socket, fileBuffer+bytesrecv, bytesleft, 0);
            //printf("received: %lu\n", temp);
            if (temp == -1)
            {
                printf("Failed to recv file\n");
                free(fileBuffer);
                fclose(file_to_transfer);
                return -1;
            }
            bytesrecv += temp;
            bytesleft -= temp;
//            sprintf(buffer, "%ld", temp);

            //printf("sending:%lu\n,",temp);
            if (send(socket, &temp, sizeof temp, 0) == -1)
            {
                printf("Failed to send bytes\n");
                free(fileBuffer);
                fclose(file_to_transfer);
                return -1;
            }
            //  clrBuf(buffer);
        }
        //printf("looping done\n");
        //printf("FileBuffer:%s\n,",fileBuffer);
        fwrite (fileBuffer , 1, bytes_to_receive, file_to_transfer);
        free(fileBuffer);
        fclose(file_to_transfer);

        //  clrBuf(buffer);
        //printf("File sent successfully\n");
        return 0;

    }
    else
    {
        free(fileBuffer);
        fclose(file_to_transfer);
        printf("Failed to open file\n");
        return -1;
    }
}
