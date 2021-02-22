/*********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_server.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2021年02月07日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月07日 14时26分41秒"
 *                 
 ********************************************************************************/
#include "ftp_server.h"
#include "ftp_server_cmd.c"

struct ARG
{
    int  client_fd;
    struct  sockaddr_in  client;
};


int main(int argc, char **argv)
{


    pthread_t   server_thread, thread;
    int   ftp_server_fd;
    int   ftp_data_fd;
    int   ftp_client_fd;
    struct sockaddr_in cliaddr;
    struct sockaddr_in servaddr;
    int           on = 1;
    socklen_t     len;
    struct ARG    arg;

    if((ftp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket failure: %s\n", strerror(errno));
        return -1;
    }

    setsockopt(ftp_server_fd, SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(FTP_SERVER_PORT);
    servaddr.sin_addr.s_addr = (INADDR_ANY);

    if(bind(ftp_server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("use bind to bind socket failure: %s\n", strerror(errno));
        return -2;
    }

    if(listen(ftp_server_fd, 64) < 0)
    {
        printf("listen failure: %s\n", strerror(errno));
        return -3;
    }

    if(pthread_create(&server_thread, NULL, server_cmd, (void *)&arg) < 0)
    {
        printf("create thread failure: %s\n", strerror(errno));
        return -4;
    }

    while(1)
    {

        printf("ftp_server> ");

        if((ftp_client_fd = accept(ftp_server_fd, (struct sockaddr *)&cliaddr, &len)) < 0)
        {
            printf("accept new client failure: %s\n", strerror(errno));
            continue;
        }

        printf("Accept new client [%d:%d] successfully\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        arg.client_fd = ftp_client_fd;
        memcpy((void *)&arg.client, &cliaddr, sizeof(cliaddr));

        if(pthread_create(&thread, NULL, client_request,(void*)&arg) < 0)
        {
            printf("crate thread failure: %s\n", strerror(errno));
            return -5;
        }
    }
    close(ftp_server_fd);

    return 0;
}

void send_client_MSG(int client_fd,char* MSG,int length)
{
    int len;
    if((len =  send(client_fd, MSG, length, 0)) < 0)
    {
        printf("send message failure: %s\n", strerror(errno));
        return ;
    }   
}

int recv_client_MSG(int client_fd)
{
    int num;

    if((num=recv(client_fd,client_Control_MSG,MAX_MSG,0))<0)
    {
        printf("receive message failure: %s\n", strerror(errno));
        return ;
    }

    client_Control_MSG[num]='\0';
    printf("Client %ld Message:%s\n",pthread_self(), client_Control_MSG);
    if(strncmp("USER", client_Control_MSG, 4) == 0||strncmp("user", client_Control_MSG, 4) == 0)
        return 2;
    return 1;
}


void *server_cmd(void *arg)
{
    char srv_cmd[SEV_MSG];

    while(1)
    {
        printf("ftp_server> ");

        fgets(srv_cmd,SEV_MSG,stdin);
        if(strncmp("exit",srv_cmd,4) == 0||strncmp("quit",srv_cmd,4)==0||strncmp("bye",srv_cmd,3)==0){
            printf("Thank you for use!\nBye!\n");
            exit(1);
        }

        if(strncmp("countcurr",srv_cmd,9)==0)
            printf("There are  %d Current active users.\n",countcurr);
        if(strncmp("countall",srv_cmd,8)==0)
            printf("There are  %d visitors\n",countall);
        if(strncmp("listname",srv_cmd,8)==0){
            printf("Sorry,code was not finished as time limited.\n");
            // printf("UserNames AS:\n");
            // for(int i=0;i<reco;i++)
            //printf("%c",username[i]);
        }
        if(strncmp("killuser",srv_cmd,8)==0){
            printf("Sorry,code was not finished as time limited.\n");//Not finish,need to add code
        }
        //   if(strncmp("help",srv_cmd,4)==0)
        //        help_information();
    }
}


void *client_request(void *arg)
{
    struct  ARG  *info;
    info = (struct ARG*)arg;
    printf("You got a connection from %d\n", inet_ntoa(info->client.sin_addr));

    do_client_work(info->client_fd, info->client);

    close(info->client_fd);
    pthread_exit(NULL);
}


int login(int client_fd)
{
    int  i=0;
    int  flag = 0;
    char  format_client_Info[MAX_MSG];

    send_client_MSG(client_fd, serverInfo220, strlen(serverInfo220));

    while(1)
    {
        if(recv_client_MSG(client_fd)==2)
            break;
        else    
            send_client_MSG(client_fd, serverInfo, strlen(serverInfo));
    }

    int length=strlen(client_Control_MSG);
    for(i=5;i<length;i++)
        format_client_MSG[i-5]=client_Control_MSG[i];
    format_client_Info[i-7]='\0';
    if(strncmp(format_client_MSG, default_user, 4) == 0)
    {
        flag=1;
    }
    if(strncmp(format_client_MSG, anony_user, 9) == 0)
    {
        flag=2;
    }

    send_client_MSG(client_fd, serverInfo331, strlen(serverInfo331));
    recv_client_MSG(client_fd);

    length=strlen(client_Control_MSG);

    for(i=5;i<length;i++)
        format_client_MSG[i-5]=client_Control_MSG[i];
    format_client_MSG[i-7]='\0';
    if(strncmp(format_client_MSG, default_pass, 4) == 0&&flag==1)
    {
        send_client_MSG(client_fd, serverInfo230, strlen(serverInfo230));
        //username[reco] = "root\n";
        //reco += 5;
        countcurr += 1;
        countall += 1;
        return 1;
    }
    else if(strncmp(format_client_Info, anony_pass, 9) == 0&&flag==2)
    {
        send_client_MSG(client_fd, serverInfo531, strlen(serverInfo531));
        //username[reco] = "anonymous\n";
        //reco += 10;
        countcurr += 1;
        countall +=1 ;
        return 2;
    }
    else
    { 
        send_client_MSG(client_fd, serverInfo530, strlen(serverInfo530));
        return 0;
    }
}

void do_client_work(int client_fd, struct sockaddr_in client)
{
    int login_flag;
    login_flag=login(client_fd);

    while(recv_client_MSG(client_fd)&&login_flag==1)
    {
        if((strncmp("quit", client_Control_MSG, 4) == 0)||(strncmp("QUIT", client_Control_MSG, 4) == 0))
        {   
            send_client_MSG(client_fd, serverInfo221, strlen(serverInfo221));
            countcurr --;
            break;
        }
        else if((strncmp("close",client_Control_MSG,5) == 0)||(strncmp("CLOSE",client_Control_MSG,5) == 0))
        {
            printf("Client Quit!\n");
            shutdown(client_fd,SHUT_WR);
            countcurr --;
        }

        else if(strncmp("pwd", client_Control_MSG, 3) == 0||(strncmp("PWD", client_Control_MSG, 3) == 0))
        {   
            char    pwd_info[SEV_MSG];
            char    tmp_dir[DIR_MSG];
            snprintf(pwd_info, SEV_MSG, "257 \"%s\" is current location.\r\n", getcwd(tmp_dir, DIR_MSG));
            send_client_MSG(client_fd, pwd_info, strlen(pwd_info));
        }
        else if(strncmp("cwd", client_Control_MSG, 3) == 0||(strncmp("CWD", client_Control_MSG, 3) == 0))
        {
            handle_cwd(client_fd);
        }
        else if(strncmp("mkd",client_Control_MSG,3) ==0||(strncmp("MKD",client_Control_MSG,3)==0))
        {
            handle_mkd(client_fd);
        }
        else if(strncmp("rmd",client_Control_MSG,3)==0||(strncmp("RMD",client_Control_MSG,3)==0))
        {
            handle_rmd(client_fd);
        }
        else if(strncmp("dele",client_Control_MSG,4)==0||(strncmp("DELE",client_Control_MSG,4)==0))
        {
            handle_del(client_fd);

        }
        else if(strncmp("pasv", client_Control_MSG, 4) == 0||(strncmp("PASV", client_Control_MSG, 4) == 0))
        {
            handle_pasv(client_fd,client);
        }
        else if(strncmp("list", client_Control_MSG, 4) == 0||(strncmp("LIST", client_Control_MSG, 4) == 0))
        {
            handle_list(client_fd);
            send_client_MSG(client_fd,serverInfo226, strlen(serverInfo226)); 
        }
        else if(strncmp("type", client_Control_MSG, 4) == 0||(strncmp("TYPE", client_Control_MSG, 4) == 0))
        {
            if(strncmp("type I", client_Control_MSG, 6) == 0||(strncmp("TYPE I", client_Control_MSG, 6) == 0))
                translate_data_mode=FILE_TRANS_MODE_BIN;
            send_client_MSG(client_fd, serverInfo200, strlen(serverInfo200));
        }
        else if(strncmp("retr", client_Control_MSG, 4) == 0||(strncmp("RETR", client_Control_MSG, 4) == 0))
        {
            handle_file(client_fd);
            send_client_MSG(client_fd,serverInfo226, strlen(serverInfo226)); 
        }
        else if(strncmp("stor", client_Control_MSG, 4) == 0||(strncmp("STOR", client_Control_MSG, 4) == 0))
        {
            handle_file(client_fd);
            send_client_MSG(client_fd,serverInfo226, strlen(serverInfo226));  
        }
        else if(strncmp("syst", client_Control_MSG, 4) == 0||(strncmp("SYST", client_Control_MSG, 4) == 0))
        {
            send_client_MSG(client_fd, serverInfo215, strlen(serverInfo215));
        }
        else if(strncmp("size", client_Control_MSG, 4) == 0||(strncmp("SIZE", client_Control_MSG, 4) == 0))
        {
            send_client_MSG(client_fd, serverInfo213, strlen(serverInfo213));
        }
        else if(strncmp("feat", client_Control_MSG, 4) == 0||(strncmp("FEAT", client_Control_MSG, 4) == 0))
        {
            send_client_MSG(client_fd, serverInfo211, strlen(serverInfo211));
        }
        else if(strncmp("rest", client_Control_MSG, 4) == 0||(strncmp("REST", client_Control_MSG, 4) == 0))
        {
            send_client_MSG(client_fd, serverInfo350, strlen(serverInfo350));
        }
        else 
        {
            send_client_MSG(client_fd, serverInfo, strlen(serverInfo));
        }

    }
    while(recv_client_MSG(client_fd)&&(login_flag == 2))
    {
        if((strncmp("quit", client_Control_MSG, 4) == 0)||(strncmp("QUIT", client_Control_MSG, 4) ==
                    0))
        {
            send_client_MSG(client_fd, serverInfo221, strlen(serverInfo221));
            countcurr --;
            break;
        }
        else if((strncmp("close",client_Control_MSG,5) == 0)||(strncmp("CLOSE",client_Control_MSG,5)
                    == 0))
        {
            printf("Client Quit!\n");
            shutdown(client_fd,SHUT_WR);
            countcurr --;
        }

        else if(strncmp("pwd", client_Control_MSG, 3) == 0||(strncmp("PWD", client_Control_MSG, 3) == 0))
        {
            char    pwd_info[SEV_MSG];
            char    tmp_dir[DIR_MSG];
            snprintf(pwd_info, SEV_MSG, "257 \"%s\" is current location.\r\n", getcwd(tmp_dir, DIR_MSG));
            send_client_MSG(client_fd, pwd_info, strlen(pwd_info));
        }
        else if(strncmp("cwd", client_Control_MSG, 3) == 0||(strncmp("CWD", client_Control_MSG, 3) == 0))
        {
            handle_cwd(client_fd);
        }
        else if(strncmp("pasv", client_Control_MSG, 4) == 0||(strncmp("PASV", client_Control_MSG, 4)
                    == 0))
        {
            handle_pasv(client_fd,client);
        }
        else if(strncmp("list", client_Control_MSG, 4) == 0||(strncmp("LIST", client_Control_MSG, 4)
                    == 0))
        {
            handle_list(client_fd);
            send_client_MSG(client_fd,serverInfo226, strlen(serverInfo226));
        }
        else if(strncmp("type", client_Control_MSG, 4) == 0||(strncmp("TYPE", client_Control_MSG, 4)
                    == 0))
        {
            if(strncmp("type I", client_Control_MSG, 6) == 0||(strncmp("TYPE I", client_Control_MSG, 6) == 0))
                translate_data_mode=FILE_TRANS_MODE_BIN;
            send_client_MSG(client_fd, serverInfo200, strlen(serverInfo200));
        }
        else if(strncmp("retr", client_Control_MSG, 4) == 0||(strncmp("RETR", client_Control_MSG, 4)
                    == 0))
        {
            handle_file(client_fd);
            send_client_MSG(client_fd,serverInfo226, strlen(serverInfo226));
        }
        else if(strncmp("syst", client_Control_MSG, 4) == 0||(strncmp("SYST", client_Control_MSG, 4)
                    == 0))
        {
            send_client_MSG(client_fd, serverInfo215, strlen(serverInfo215));
        }
        else if(strncmp("size", client_Control_MSG, 4) == 0||(strncmp("SIZE", client_Control_MSG, 4)
                    == 0))
        {
            send_client_MSG(client_fd, serverInfo213, strlen(serverInfo213));
        }
        else if(strncmp("feat", client_Control_MSG, 4) == 0||(strncmp("FEAT", client_Control_MSG, 4)
                    == 0))
        {
            send_client_MSG(client_fd, serverInfo211, strlen(serverInfo211));
        }
        else if(strncmp("rest", client_Control_MSG, 4) == 0||(strncmp("REST", client_Control_MSG, 4)
                    == 0))
        {
            send_client_MSG(client_fd, serverInfo350, strlen(serverInfo350));
        }
        else
        {
            send_client_MSG(client_fd, serverInfo, strlen(serverInfo));
        }

    }
}

/* 
   void help_information()
   {
   printf("55kai  niubi\n");
   }
   */
