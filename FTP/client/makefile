
all:ftp_client_cmd.o ftp_client.o
	gcc ./*.o -o ftp_client

ftp_client.o:ftp_client.c
	gcc -c ftp_client.c -o ftp_client.o

ftp_client_cmd.o:ftp_client_cmd.c
	gcc -c ftp_client_cmd.c -o ftp_client_cmd.o

clean:
	rm -rf *.o ftp_client

