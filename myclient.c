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

#define BUF 1024

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
int main (int argc, char **argv)
{
    int login=1;//0 is logged in
    int create_socket;
    char buffer[BUF];
    struct sockaddr_in address;
    int size;
    char file_name[BUF];
    char file_path[BUF];
    char file_name_helper[BUF];
    long port;
    if( argc < 3 )
    {
        printf("Usage: %s Address Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if ((create_socket = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error");
         return EXIT_FAILURE;
    }
    port=castPortToLong(argv[2]);

    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (port);
    inet_aton (argv[1], &address.sin_addr);

    if (connect ( create_socket, (struct sockaddr *) &address, sizeof (address)) == 0)
    {
        printf ("Connection with server (%s) established\n", inet_ntoa (address.sin_addr));
        size=recvString(buffer,create_socket);
         if(size==-1){
             return EXIT_FAILURE;
        }
        //size=recv(create_socket,buffer,BUF-1, 0);
        // if (size>0)
        // {
        //     buffer[size]= '\0';
        //     printf("%s",buffer);
        // }
    }
    else
    {
        perror("Connect error - no server available");
        return EXIT_FAILURE;
    }
    // int banchk;
    // recvInt(&banchk,create_socket);
    //     if(banchk==2){
    //       printf ("Your are still banned\n");
    //       return EXIT_SUCCESS;
    //     }

    do
    {
      // while(login==1){
      //   if(login==1){
      //     printf ("Login please:\n");
      //     fgets (buffer, BUF, stdin);
      //   }
      // }
        clrBuf(buffer);
        printf ("Send message: ");
        fgets (buffer, BUF, stdin);
        // send(create_socket, &commandsize, sizeof commandsize, 0);

        // send(create_socket, buffer, strlen (buffer), 0);
        sendString(buffer,create_socket);
        if(login==1){
        if(!startsWith("login",buffer)&&(strncmp (buffer, "quit", 4)  != 0)&&(strncmp (buffer, "QUIT", 4)  != 0)&&(strncmp (buffer, "Quit", 4)  != 0)){
            printf("You are not logged in\n" );
            continue;
        }
      }
        if(strcmp (buffer, "list\n") == 0 || strncmp(buffer, "LIST",4) == 0 || strncmp(buffer, "List",4) == 0)
        {
            clrBuf(buffer);
            //recv(create_socket,&commandsize,sizeof commandsize,0);

            //size = recv (create_socket, buffer, commandsize, 0);
            if(size=recvString(buffer,create_socket)==-1){
                return EXIT_FAILURE;
            }

//            size=recv(create_socket,buffer,BUF-1, 0);
            printf ("Files:\n------------------------------------------------------\n%s", buffer);
            printf ("------------------------------------------------------\n");
        }
        else if (startsWith("get",buffer)||startsWith("GET",buffer)||startsWith("Get",buffer))
        {
            memset(file_name,'\0',sizeof file_name);
            strncpy(file_name,&buffer[4],strlen(buffer)-4);
            //printf("fnf:%s \n",file_name);
            //printf("fn2:%s \n",file_name);
            strcpy(file_path,"./");
            file_name[strcspn(file_name, "\n")] = '\0';
            //printf("fna:%s \n",file_path);
            if(recvFile(create_socket,file_name, file_path,1)==0){
                printf("File %s successfully received\n",file_name);
            }
        }
        else if (startsWith("put",buffer)||startsWith("PUT",buffer)||startsWith("Put",buffer))
        {

            memset(file_name,'\0',sizeof file_name);
            strncpy(file_name,&buffer[4],strlen(buffer)-4);
            //printf("%s \n",file_name);

            file_name[strcspn(file_name, "\n")] = '\0';
            strcpy(file_name_helper,file_name);
            strcpy(file_name,"./");
            strcat(file_name,file_name_helper);
            //printf("%s \n",file_name);
            if(sendFile(create_socket,file_name)==0){
                printf("File %s successfully sent\n",file_name);
            }
        }else if(startsWith("login",buffer)){
            //login=1
            // int attempts;
            // recvInt(&login,create_socket);
            // recvInt(&attempts,create_socket);
            // if(login==0){
            //   continue;
            // }
            // if(attempts==0){
            //   return EXIT_FAILURE;
            // }
            // clrBuf(buffer);
            // printf ("Username:");
            // fgets (buffer, BUF, stdin);
            // sendString(buffer,create_socket);
            // clrBuf(buffer);
            //fgets (buffer, BUF, stdin);
            //sendString(buffer,create_socket);


            int attempts;
            recvInt(&login,create_socket);
            recvInt(&attempts,create_socket);
            if(login==1){
              printf ("Login please:");
              clrBuf(buffer);
              scanf("%s", buffer);
              //fgets (buffer, BUF, stdin);
              sendString(buffer,create_socket);
              char *pw = getpass("\nPassword:");
              sendString(pw,create_socket);
              int ldapvalue;
              recvInt(&ldapvalue,create_socket);
              printf("%i\n", ldapvalue);
              if (ldapvalue==0){
                  login=0;
                  printf("Logged in\n");
              }else if(ldapvalue==1){
                  printf("User does not exist\n");
              }else if(ldapvalue==2){
                  printf("Passwords does not match\n");
              }else{
                  printf("LDAP Error\n");
              }
            }else if(login==0){
                printf("Already logged in\n");

            } else if(attempts==0){
                char *banmsg;
                recvString(banmsg,create_socket);
                printf("%s\n",banmsg);
            }else{

            }
        }
        printf("\n");
    }
    while ((strncmp (buffer, "quit", 4)  != 0)&&(strncmp (buffer, "QUIT", 4)  != 0)&&(strncmp (buffer, "Quit", 4)  != 0));
    close (create_socket);
    return EXIT_SUCCESS;
}
