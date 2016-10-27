all: myserver myclient
	rm ../f.h ./f.b
myclient: myclient.o fileTransmissionHelper.o
	gcc myclient.o fileTransmissionHelper.o -o myclient

myclient.o: myclient.c
	gcc -c myclient.c

myserver: myserver.o fileTransmissionHelper.o
	gcc myserver.o fileTransmissionHelper.o -o myserver

myserver.o: myserver.c
	gcc -c myserver.c

fileTransmissionHelper.o: fileTransmissionHelper.c fileTransmissionHelper.h
	gcc -c fileTransmissionHelper.c

clean: 
	rm -f myserver myclient myserver.o myclient.o fileTransmissionHelper.o
	
