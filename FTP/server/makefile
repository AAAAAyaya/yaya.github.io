//CC := gcc

all:ftp_server.o ftp_server_cmd.o
	gcc ./*.o -o ftp_server

ftp_server.o:ftp_server.c
	gcc -c ftp_server.c -o ftp_server.o -lpthread

ftp_server_cmd.o:ftp_server_cmd.c
	gcc -c ftp_server_cmd.c -o ftp_server_cmd.o

clean:
	rm -rf *.o ftp_server

