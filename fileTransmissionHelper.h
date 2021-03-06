#ifndef FILEHELPER_H_INCLUDED
#define FILEHELPER_H_INCLUDED
void clrBuf(char *buffer);

int recvFile(int socket, char *file_name, char *file_path,int showprogress);

int sendFile(int socket, char *file_name);

int castPortToLong(char *port);

int receive_int(int *num, int fd);

int send_int(int num, int fd);

int sendString(char *buffer,int socket);

int recvString(char *buffer,int socket);

int sendInt(int value,int socket);

int recvInt(int *value,int socket);

#endif
