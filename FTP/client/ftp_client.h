/********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_client.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2021年02月3日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月3日 00时12分52秒"
 *                 
 ********************************************************************************/
#ifndef  _FTP_H_
#define  _FTP_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <curses.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_FTP_PORT 21


int fill_host_addr(char * host_ip_addr, struct sockaddr_in * host, int port);


int uconnect(struct sockaddr_in *s_addr, int type);//connect to ftp server


void echo_off();//hide information on the terminal


void echo_on();//display information on the terminal


void get_username();//get user name from the terminal


void get_passwd();//get pass word from the terminal


int ftp_login();//log in


int ftp_send_cmd(const char *s1, const char *s2, int sock_fd);//send command to server with socket


int ftp_get_reply(int sock_fd);//get the server's reply message with socket


int get_port();


int rand_local_port();//get random local port for user


void ftp_list();//ls command


int uconnect_ftpdata();


void ftp_cmd_filename(char * usr_cmd, char * src_file, char * dst_file);


void ftp_get(char * usr_cmd);//get


void ftp_put(char * usr_cmd);//put


void ftp_quit();//bye


void close_cli();//close


void ftp_pwd();//pwd


void ftp_cd();//cd


void ftp_del(char *usr_cmd);//dele


void mkdir_srv(char *usr_cmd);//mkdir


void rmdir_srv(char *usr_cmd);//rmdir


void local_list();//lls


void local_pwd();//lpwd


void local_cd(char * usr_cmd);//lcd


int ftp_usr_cmd(char * usr_cmd);


void open_srv();


int start_ftp_cmd(char * host_ip_addr, int port);


void open_ftpsrv();


void show_help();



int ftp_mode = 1;
int sock_control;
extern int h_errno;

struct sockaddr_in ftp_server, local_host;
char user[64]; //ftp usr
char passwd[64]; //ftp passwd
struct hostent * server_hostent;

#endif
