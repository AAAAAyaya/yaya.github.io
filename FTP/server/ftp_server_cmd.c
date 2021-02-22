/*********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_server_cmd.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2021年02月2日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月2日 16时14分48秒"
 *                 
 ********************************************************************************/
#include "ftp_server.h"



void handle_cwd(int client_sock)
{
    char    cwd_info[MAX_MSG];
    char    tmp_dir[DIR_MSG];
    char    client_dir[DIR_MSG];

    char t_dir[DIR_MSG];
    int  dirlength=-1;
    int  length=strlen(client_Control_MSG);
    int  i=0;    
    for(i=4;i<length;i++)
        format_client_MSG[i-4]=client_Control_MSG[i];
    format_client_MSG[i-6]= '\0';

    if(strncmp(getcwd(t_dir, DIR_MSG),format_client_MSG,strlen(getcwd(t_dir, DIR_MSG))-10)!=0)
    {
        getcwd(client_dir, DIR_MSG);
        dirlength=strlen(client_dir);
        client_dir[dirlength]='/';
    }


    for(i=4;i<length;i++)
    {
        client_dir[dirlength+i-3]=client_Control_MSG[i];
    }
    client_dir[dirlength+i-5]='\0';

    printf("%s\r\n",client_dir);
    //printf("%s\r\n",client_Control_MSG);   
    //printf("%s\r\n",getcwd(t_dir, DIR_MSG));   

    if (chdir(client_dir) >= 0)
    {
        snprintf(cwd_info, MAX_MSG, "257 \"%s\" is current location.\r\n", getcwd(tmp_dir, DIR_MSG));
        send_client_MSG(client_sock, cwd_info, strlen(cwd_info));
    }
    else
    {
        snprintf(cwd_info, MAX_MSG, "550 %s :%s\r\n",client_dir,strerror(errno));
        perror("chdir():");
        send_client_MSG(client_sock, cwd_info, strlen(cwd_info));
    }               
}


//error
void handle_rmd(int client_sock)
{
    char    rmd_info[MAX_MSG];
    char    tmp_dir[DIR_MSG];
    char    client_dir[DIR_MSG];

    char t_dir[DIR_MSG];
    int dirlength=-1;
    int length=strlen(client_Control_MSG);
    int i=0;
    for(i=4;i<length;i++)
        format_client_MSG[i-4]=client_Control_MSG[i];
    format_client_MSG[i-6]='\0';

    if(strncmp(getcwd(t_dir, DIR_MSG),format_client_MSG,strlen(getcwd(t_dir, DIR_MSG))-10)!=0)
    {
        client_dir[dirlength]='/';
    }


    for(i=4;i<length;i++)
    {
        client_dir[dirlength+i-3]=client_Control_MSG[i];
    }
    client_dir[dirlength+i-5]='\0';

    printf("%s\r\n",client_dir);
    if
        (rmdir(client_dir) >= 0)
        {
            printf( " \"%s\" is deleted successfully.\r\n", client_dir);                
            send_client_MSG(client_sock, rmd_info, strlen(rmd_info));
        }
    else
    {
        snprintf(rmd_info, MAX_MSG, "550 %s :%s\r\n",client_dir,strerror(errno));
        printf("rmdir failure:%s\n", strerror(errno));
        send_client_MSG(client_sock, rmd_info, strlen(rmd_info));
    }
}


void handle_mkd(int client_sock)
{
    char    mkd_info[MAX_MSG];
    char    tmp_dir[DIR_MSG];
    char    client_dir[DIR_MSG];

    char t_dir[DIR_MSG];
    int dirlength=-1;
    int length=strlen( client_Control_MSG);
    int i=0;
    for(i=4;i<length;i++)
        format_client_MSG[i-4]=client_Control_MSG[i];
    format_client_MSG[i-6]='\0';
    if(strncmp(getcwd(t_dir, DIR_MSG),format_client_MSG,strlen(getcwd(t_dir, DIR_MSG))-10)!=0)
    {
        getcwd(client_dir, DIR_MSG);
        dirlength=strlen(client_dir);
        client_dir[dirlength]='/';
    }

    for(i=4;i<length;i++)
    {
        client_dir[dirlength+i-3]=client_Control_MSG[i];
    }
    client_dir[dirlength+i-5]='\0';

    printf("%s\r\n",client_dir);
    if (mkdir(client_dir,0644) >= 0)
    {
        printf( " \"%s\" is created successfully.\r\n", client_dir);
        send_client_MSG(client_sock, mkd_info, strlen(mkd_info));
    }
    else
    {
        snprintf(mkd_info, MAX_MSG, "550 %s :%s\r\n",client_dir,strerror(errno));
        printf("mkdir failure:%s\n", strerror(errno));
        send_client_MSG(client_sock, mkd_info, strlen(mkd_info));
    }
}

void handle_list(int client_sock)
{
    send_client_MSG(client_sock, serverInfo150, strlen(serverInfo150));

    int t_data_sock;
    struct sockaddr_in client;
    int sin_size=sizeof(struct sockaddr_in);
    if((t_data_sock=accept(ftp_data_sock,(struct sockaddr *)&client,&sin_size))==-1)
    {
        printf("accept failure: %s\n", strerror(errno));
        return;
    }

    FILE *pipe_fp;
    char t_dir[DIR_MSG];
    char list_cmd_info[DIR_MSG];
    snprintf(list_cmd_info, DIR_MSG, "ls -l %s", getcwd(t_dir, DIR_MSG));

    if ((pipe_fp = popen(list_cmd_info, "r")) == NULL)
    {
        printf("pipe open error in cmd_list\n");
        return ;
    }
    printf("pipe open successfully!, cmd is %s\n", list_cmd_info);

    char t_char;
    while ((t_char = fgetc(pipe_fp)) != EOF)
    {
        printf("%c", t_char);
        write(t_data_sock, &t_char, 1);
    }
    pclose(pipe_fp);
    printf("close pipe successfully!\n");   
    close(t_data_sock); 
    printf("%s close data successfully!\n",serverInfo226);    
    close(ftp_data_sock);   
}


void handle_pasv(int client_sock,struct sockaddr_in client)
{

    char pasv_msg[MAX_MSG];
    char    port_str[8];
    char    addr_info_str[30];
    struct sockaddr_in user_data_addr;
    user_data_addr=create_date_sock();

    int     tmp_port1;
    int      tmp_port2;
    tmp_port1 = ntohs(user_data_addr.sin_port) / 256;
    tmp_port2 = ntohs(user_data_addr.sin_port) % 256;
    long ipNum=inet_addr(inet_ntoa(client.sin_addr));
    printf("%s  %d\r\n",inet_ntoa(client.sin_addr),ipNum);

    snprintf(addr_info_str, sizeof(addr_info_str), "%ld,%ld,%ld,%ld,", ipNum&0xff,
            ipNum>>8&0xff,
            ipNum>>16&0xff,
            ipNum>>24&0xff);
    snprintf(port_str, sizeof(port_str), "%d,%d", tmp_port1, tmp_port2);
    strcat(addr_info_str, port_str);
    snprintf(pasv_msg, MAX_MSG, "227 Entering Passive Mode (%s).\r\n", addr_info_str);
    send_client_MSG(client_sock, pasv_msg, strlen(pasv_msg));

}


//create data sock;

struct sockaddr_in create_date_sock( )
{

    int t_client_sock;
    struct sockaddr_in t_data_addr;
    t_client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (t_client_sock < 0)
    {
        printf("create data socket error!\n");
        return;
    }
    srand((int)time(0));
    int a=rand()%1000+1025;
    bzero(&t_data_addr,sizeof(t_data_addr));
    t_data_addr.sin_family =AF_INET;
    t_data_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    t_data_addr.sin_port = htons(a);     

    if(bind(t_client_sock,(struct sockaddr*)&t_data_addr,sizeof(struct sockaddr)) <0)
    {
        printf("bind error in create data socket:%s\n",strerror(errno));
        return;
    }
    listen(t_client_sock, 64);

    ftp_data_sock=t_client_sock;
    return      t_data_addr;
}


void handle_file(int client_sock)
{
    send_client_MSG(client_sock, serverInfo150, strlen(serverInfo150));

    int t_data_sock;
    struct sockaddr_in client;
    int sin_size=sizeof(struct sockaddr_in);
    if((t_data_sock=accept(ftp_data_sock,(struct sockaddr *)&client,&sin_size))==-1)
    {
        perror("accept error");
        return;
    }
    int i=0;
    int length=strlen(client_Control_MSG);
    for(i=5;i<length;i++)
        format_client_MSG[i-5]=client_Control_MSG[i];
    format_client_MSG[i-7]='\0';


    FILE* fp;
    int file_fd;
    int n;
    char t_dir[DIR_MSG];

    char file_info[DIR_MSG];

    snprintf(file_info, DIR_MSG, "%s/%s", getcwd(t_dir, DIR_MSG),format_client_MSG);

    printf("%s\r\n",file_info);
    //printf("%s\r\n",format_client_Info);    
    //printf("%s\r\n",getcwd(t_dir, DIR_MSG)); 
    char file_mode[3];
    if(strncmp("retr", client_Control_MSG, 4) == 0||(strncmp("RETR", client_Control_MSG, 4) == 0))
    {
        file_mode[0]='r';
        file_mode[1]='b';
        file_mode[2]='\0';
    }
    else  
    {
        file_mode[0]='a';
        file_mode[1]='b';
        file_mode[2]='\0';
    }

    if(strncmp(getcwd(t_dir, DIR_MSG),format_client_MSG,strlen(getcwd(t_dir, DIR_MSG))-1)==0)
        fp = fopen(format_client_MSG, file_mode);
    else
        fp = fopen(file_info, file_mode);

    if (fp == NULL)
    {
        printf("open file error:%s\r\n",strerror(errno));
        char    cwd_info[MAX_MSG];
        snprintf(cwd_info, MAX_MSG, "550 %s :%s\r\n",format_client_MSG,strerror(errno));
        send_client_MSG(client_sock, cwd_info, strlen(cwd_info));
        close(t_data_sock);
        close(ftp_data_sock);   
        return;
    }
    int cmd_sock=fileno(fp);
    memset(client_Data_MSG, 0, MAX_MSG);    
    if(strncmp("retr", client_Control_MSG, 4) == 0||(strncmp("RETR", client_Control_MSG, 4) == 0))
    {
        while ((n = read(cmd_sock, client_Data_MSG, MAX_MSG)) > 0)
        {
            if (write(t_data_sock, client_Data_MSG, n) != n)
            {
                printf("retr transfer error\n");
                return;
            }
        }
    }
    else /* if(strncmp("stor",client_Control_MSG,4)==0||(strncmp("STOR",client_Control_MSG,4)==0))*/
    {
        while ((n = read(t_data_sock, client_Data_MSG, MAX_MSG)) > 0)
        {
            if (write(cmd_sock, client_Data_MSG, n) != n)
            {
                printf("stor transfer error\n");
                return;
            }
        }
    }

    fclose(fp);
    close(t_data_sock);
    close(ftp_data_sock); 
}





void handle_del(int client_sock)
{

    char    del_info[MAX_MSG];
    char    tmp_file[DIR_MSG];
    char    client_dir[DIR_MSG];

    char t_dir[DIR_MSG];
    int dirlength=-1;
    int length=strlen(client_Control_MSG);
    int i=0;
    for(i=4;i<length;i++)
        format_client_MSG[i-4]=client_Control_MSG[i];
    format_client_MSG[i-6]='\0';

    if(strncmp(getcwd(t_dir, DIR_MSG),format_client_MSG,strlen(getcwd(t_dir, DIR_MSG))-10)!=0)
    {
        getcwd(client_dir, DIR_MSG);
        dirlength=strlen(client_dir);
        client_dir[dirlength]='/';
    }


    for(i=4;i<length;i++)
    {
        client_dir[dirlength+i-3]=client_Control_MSG[i];
    }
    client_dir[dirlength+i-5]='\0';
    printf("%s\r\n",client_dir);
    if (unlink(client_dir) >= 0)
    {
        printf( " \"%s\" is deleted successfully.\r\n", client_dir);
        send_client_MSG(client_sock, del_info, strlen(del_info));
    }
    else
    {
        snprintf(del_info, MAX_MSG, "550 %s :%s\r\n",client_dir,strerror(errno));
        printf("unlink failure:%s\n", strerror(errno));
        send_client_MSG(client_sock, del_info, strlen(del_info));
    }
}




