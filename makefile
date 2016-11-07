all: myserver myclient

myclient: myclient.o fileTransmissionHelper.o
	gcc myclient.o fileTransmissionHelper.o -o myclient

myclient.o: myclient.c
	gcc -c myclient.c

myserver: myserver.o fileTransmissionHelper.o
	gcc -DLDAP_DEPRECATED myserver.o fileTransmissionHelper.o -o myserver -lldap -llber

myserver.o: myserver.c
	gcc -c myserver.c 

fileTransmissionHelper.o: fileTransmissionHelper.c fileTransmissionHelper.h
	gcc -c fileTransmissionHelper.c

clean: 
	rm -f myserver myclient myserver.o myclient.o fileTransmissionHelper.o
	
