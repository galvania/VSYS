#ifndef FILEHELPER_H_INCLUDED
#define FILEHELPER_H_INCLUDED
void clrBuf(char *buffer);

int recvFile(int socket, char *file_name, char *file_path);

int sendFile(int socket, char *file_name);

int castPortToLong(char *port);

int sendString(char *buffer,int socket);

int recvString(char *buffer,int socket);
#endif
