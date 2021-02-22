/********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_server.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2021年02月07日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月07日 14时14分41秒"
 *                 
 ********************************************************************************/

#ifndef __FTP_H_
#define __FTP_H_

#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define FTP_SERVER_PORT  21
#define COMMAND_NUM 7
#define FILE_TRANS_MODE_ASIC 0
#define FILE_TRANS_MODE_BIN 1
#define MAX_SIZE    1024
#define SEV_MSG   100
#define MAX_MSG    1024
#define DIR_MSG   100

void do_client_work(int client_sock,struct sockaddr_in client);
int login(int client_sock);
void handle_cwd(int client_sock);
void handle_list(int client_sock);
void handle_pasv(int client_sock,struct sockaddr_in client);
void handle_file(int client_sock);
void handle_del(int client_sock);
void handle_mkd(int client_sock);
void handle_rmd(int client_sock);
struct sockaddr_in create_date_sock();

  
void send_client_MSG(int client_sock,char* info,int length);
int recv_client_MSG(int client_sock);
void *client_request(void *arg);
void *server_cmd(void *arg);
int countcurr = 0;
int countall = 0;
char  client_Control_MSG[MAX_MSG];
char  client_Data_MSG[MAX_MSG];
char  format_client_MSG[MAX_MSG];
int translate_data_mode=FILE_TRANS_MODE_ASIC;
int  ftp_server_sock;
int  ftp_data_sock;
 

char    default_user[] = "root";
char    default_pass[] = "1234";
char    anony_user[]="anonymous";
char    anony_pass[]="anonymous";



char  serverInfo220[]="220 myFTP Server ready...\r\n";
char serverInfo230[]="230 User logged in, proceed.\r\n";
char serverInfo331[]="331 User name okay, need password.\r\n";
char serverInfo221[]="221 Goodbye!\r\n";
char serverInfo150[]="150 File status okay; about to open data connection.\r\n";
char serverInfo226[]="226 Closing data connection.\r\n";
char serverInfo200[]="200 Command okay.\r\n";
char serverInfo215[]="215 Unix Type FC5.\r\n";
char serverInfo213[]="213 File status.\r\n";
char serverInfo211[]="211 System status, or system help reply.\r\n";
char serverInfo350[]="350 Requested file action pending further information.\r\n";
char serverInfo530[]="530 Not logged in.\r\n";
char serverInfo531[]="531 Not root client. Anonymous client.\r\n";
char serverInfo[]="202 Command not implemented, superfluous at this site.\r\n";

#endif



