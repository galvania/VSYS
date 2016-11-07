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
#include <ldap.h>

#include "fileTransmissionHelper.h"
#define BUF 1024
#define PORT 6543
#define FILE_TO_SEND    "/home/schoko/Downloads/srv/"

int ldaplogin(char *user, char *pass)
{
  char *ldap_host = "ldap.technikum-wien.at";

int ldap_port = 389;

LDAP *ldap;

LDAPMessage *result, *entry;

int returnCode;

char *search, *dn;



if((ldap = ldap_init(ldap_host, ldap_port)) == NULL)

{

    perror("ldap_init failed");

    return -1;

}

//printf("Connected to LDAP server %s on port %d\n", ldap_host, ldap_port);



// anonymous bind

returnCode = ldap_simple_bind_s(ldap, NULL, NULL);

if(returnCode != LDAP_SUCCESS)

{

    ldap_unbind(ldap);

    fprintf(stderr,"LDAP error: %s\n", ldap_err2string(returnCode));

    return -1;

}



search = malloc(snprintf(NULL, 0, "(uid=%s)", user)+1);

sprintf(search, "(uid=%s)", user);

returnCode = ldap_search_s(ldap, "ou=people,dc=technikum-wien,dc=at", LDAP_SCOPE_ONELEVEL, search, NULL, 0, &result);

if(returnCode != LDAP_SUCCESS)

{

    ldap_unbind(ldap);

    fprintf(stderr,"LDAP search error: %s\n", ldap_err2string(returnCode));

    return -1;

}



entry = ldap_first_entry(ldap, result);
printf("search %s\n",entry);
if(entry == NULL) {

    ldap_unbind(ldap);

    ldap_msgfree(result); //free memory

    return 1;

}

dn = ldap_get_dn(ldap, entry);

ldap_msgfree(result); //free memory



// bind with credentials

returnCode = ldap_simple_bind_s(ldap, dn, pass);

if(returnCode != LDAP_SUCCESS)

{

    ldap_unbind(ldap);

    if(returnCode == 49)

        return 2;

    fprintf(stderr,"LDAP error: %s\n", ldap_err2string(returnCode));

    return -1;

}



ldap_unbind(ldap);

return 0;
}
int main (int argc, char **argv)
{
    int create_socket, new_socket;
    socklen_t addrlen;
    char buffer[BUF];
    char listbuffer[BUF];
    int size;
    struct sockaddr_in address, cliaddress;
    int login=1; //0 is logged in
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
            strcpy(buffer,"Welcome to myserver, Please Login:\n");
            sendString(buffer,new_socket);
        }
        int attempts=3;
        do
        {

            clrBuf(buffer);
            //if still banned
            //printf("Send ban %i\n",sendInt(2,new_socket));
            //exit(3);
            //else
            //sendInt(1,new_socket);
            //sendString("Your are still banned for");
          //   while(attempts>0&&login>0){
          //     if(login==1){
          //       char name[BUF];
          //       char pass[BUF];
          //       size = recvString(name,new_socket);
          //       size = recvString(pass,new_socket);
          //       //++Password Check
          //       login==0;
          //       //++onBan
          //       login==2;
          //       if(login==0){
          //         sendInt(login,new_socket);
          //       } else if(login==2) {
          //         sendInt(login,new_socket);
          //         sendString("You are banned after 3 failed attempts to login",new_socket);
          //         exit(3);
          //       }else {
          //         sendInt(login,new_socket);
          //         attempts--;
          //       }
          //       printf("Login successful");
          //       if(login==1){
          //         continue;
          //       }
          //     }
          // }
            size = recvString(buffer,new_socket);
            if( size > 0)
            {

//                buffer[size] = '\0';
                printf ("Message received: %s\n", buffer);
                //LIST
                if(login==1){
                    if(strncmp(buffer, "login",5)  != 0){
                        continue;
                    }
                }
                if (strncmp(buffer, "list",4) == 0 || strncmp(buffer, "LIST",4) == 0 || strncmp(buffer, "List",4) == 0)
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
                        strcat(listbuffer,"Not a valid Directory\n");
                        perror ("Error while opening the Folder\n");
                        //return EXIT_FAILURE;
                    }
                    sendString(listbuffer,new_socket);
                }
                //GET
                else if(strncmp(buffer, "get",3)  == 0 ||strncmp(buffer, "GET",3)  == 0||strncmp(buffer, "Get",3)  == 0)
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
                else if(strncmp(buffer, "put",3)  == 0 ||strncmp(buffer, "PUT",3)  == 0||strncmp(buffer, "Put",3)  == 0)
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
                else if(strncmp(buffer, "login",5)  == 0)
                {
                    //int login=1; //0 is logged in
                    // sendInt(login,new_socket);
                    // sendInt(attempts,new_socket);
                    // if(login==0){
                    //   continue;
                    // }
                    // if(attempts==0){
                    //   return EXIT_FAILURE;
                    // }
                    // char *username;
                    // char *password;
                    // size = recvString(buffer,new_socket);
                    // printf("user: %s ",buffer);
                    //recvString(password,new_socket);
                    //printf("user: %s pass: %s",username,password);


                    char user[BUF];
                    char pass[BUF];
                    printf("sendInt\n" );
                    if(sendInt(login,new_socket)){
                      perror("Failed at sending login\n");
                    }
                    printf("sendInt\n" );
                    if(sendInt(attempts,new_socket)!=0){
                      perror("Failed at sending attempts\n");
                    }
                    printf("pass\n");
                    if(login==0){
                        printf("Logged in \n" );
                      continue;
                    }
                    if(attempts==0){
                        printf("sendString\n" );
                        char *banmsg ="You're banned for idk much time";
                        sendString(banmsg,new_socket);
                        continue;
                    }
                    int err;

                    err=recvString(user,new_socket);
                    printf("User:%s\nLength:%i\n", user,strlen(user));
                    recvString(pass,new_socket);
                    int ldapvalue;
                    ldapvalue = ldaplogin(user,pass);
                    printf("ldap:%i\n",ldapvalue);
                    if(ldapvalue==0){
                        login=0;
                        sendInt(ldapvalue,new_socket);
                    }else if(ldapvalue==1){
                        sendInt(ldapvalue,new_socket);
                        attempts--;
                    }else if(ldapvalue==2){
                        sendInt(ldapvalue,new_socket);
                        attempts--;
                    }else{
                        sendInt(ldapvalue,new_socket);
                        attempts--;
                    }
                }
//                clrBuf(buffer);
            }
            else if (size == 0)
            {
                printf("Client closed remote socket\n");
                exit(3);
                break;
            }
            else
            {
                printf("Client closed remote socket\n");
                exit(3);
                return EXIT_SUCCESS;
            }

        }
        while ((strncmp (buffer, "quit", 4)  != 0)&&(strncmp (buffer, "QUIT", 4)  != 0)&&(strncmp (buffer, "Quit", 4)  != 0));
        if (pid == 0){
            printf("Child closed\n");
            exit(3);
            return EXIT_SUCCESS;
        }
        close (new_socket);
    }
    close (create_socket);

    return EXIT_SUCCESS;
}
