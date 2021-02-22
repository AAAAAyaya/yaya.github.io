/*********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2021年02月3日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月3日 00时12分20秒"
 *                 
 ********************************************************************************/
#include "ftp_client.h"
#include "ftp_client_cmd.c"

/*
int ftp_mode = 1;
int sock_control;
extern int h_errno;

struct sockaddr_in ftp_server, local_host;
char user[64]; //ftp usr
char passwd[64]; //ftp passwd
struct hostent * server_hostent;
*/
int main(int argc, char *argv[])
{
    int sock_control;
    if(argc == 1)
    {   
        printf("   Welcome to our FTP v1.0 by 55open   \n");
        printf("   You are offered the following operations: \n");
        printf("    open, close, mode, get, put, bye, ls, pwd, cd, dele, mkdir, rmdir, lls, lcd, lpwd\n");
        open_ftpsrv();

    }
    else
    {
        if(argc == 2 && argc == 3)
        {
            if(argv[2]==NULL)
                start_ftp_cmd(argv[1], DEFAULT_FTP_PORT);
            else
                start_ftp_cmd(argv[1], atol(argv[2]));
        }
        else
        {
            printf("INPUT INVIABLE!\n");
            exit(-1);
        }
    }
    return 1;
}


/*Function that sends the client's command to the server*/

int ftp_send_cmd(const char *s1, const char *s2, int sock_fd)
{
    char send_buf[256];
    int send_err, len;
    if(s1) 
    {
        strcpy(send_buf,s1);
        if(s2)
        {   
            strcat(send_buf, s2);
            strcat(send_buf,"\r\n");
            len = strlen(send_buf);
            send_err = send(sock_fd, send_buf, len, 0);
        }
        else 
        {
            strcat(send_buf,"\r\n");
            len = strlen(send_buf);
            send_err = send(sock_fd, send_buf, len, 0);
        }
    }
    if(send_err < 0)
        printf("*****ERROR: send() error*****\n");
    return send_err;
}


/*get the server's reply message from sock_fd*/
int ftp_get_reply(int sock_fd)
{
    static int reply_code = 0,count=0;
    char recv_buf[512];
    count=read(sock_fd, recv_buf, 510);
    if(count > 0)
        reply_code = atoi(recv_buf);
    else
        return 0;
    while(1)
    {
        if(count <= 0)
            break;
        recv_buf[count]='\0';
        printf("%s",recv_buf);
        count=read(sock_fd, recv_buf, 510);
    }
    return reply_code;
}



/*functions for client operation*/
void open_ftpsrv()
{
    char usr_cmd[1024];
    int cmd_flag;
    while(1)
    {
        printf("ftp_client>");
        fgets(usr_cmd,510,stdin);
        fflush(stdin);
        if(usr_cmd[0] == '\n')
            continue;
        usr_cmd[strlen(usr_cmd)-1] = '\0';
        cmd_flag = ftp_usr_cmd(usr_cmd);
        if(cmd_flag == 15)
        {
            char *cmd = strchr(usr_cmd,' ');
            char dress_ftp[1024];
            if(cmd == NULL)
            {
                printf("*****ERROR: command error*****\n");
                show_help();
                return;
            }
            else
            {
                while(*cmd == ' ')
                    cmd++;
            }
            if(cmd == NULL||cmd == '\0')
            {
                printf("*****ERROR: command error*****\n");
                return;
            }
            else
            {
                char * dr = "127.0.0.1";
                strncpy(dress_ftp,cmd,strlen(cmd));
                dress_ftp[strlen(cmd)] = '\0';
                printf("%s",dress_ftp);
                if(strncmp(dress_ftp,dr,9) == 0)
                {
                    printf("Connect Successed!\n");
                //    if succeed connecting to server, then call start_ftp_cmd
                        start_ftp_cmd(dr,DEFAULT_FTP_PORT);
                }
                else
                {
                    printf("Inviable Server Dress!\n");
                }   

            }
        }
        else//If open is not successful, it can only perform operations on the client side, unable to perform server operations.
        {
            switch(cmd_flag)
            {
                case 11:
                    local_list();
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
                case 12:
                    local_pwd();
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
                case 13:
                    local_cd(usr_cmd);
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
                case 6://quit
                    printf("BYE!\n");
                printf("Thank you for use!\n");
                exit(0);
                break;
                default:
                printf("*****ERROR:command error*****\n");
                show_help();
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
            }
        }
    }       

}




void open_srv()
{
    char usr_cmd[1024];
    int cmd_flag;
    while(1)
    {
        printf("ftp_client>");
        fgets(usr_cmd,510,stdin);
        fflush(stdin);
        if(usr_cmd[0] == '\n')
            continue;
        usr_cmd[strlen(usr_cmd)-1] = '\0';
        cmd_flag = ftp_usr_cmd(usr_cmd);
        if(cmd_flag == 15)
        {
            char *cmd = strchr(usr_cmd,' ');
            char dress_ftp[1024];
            if(cmd == NULL)
            {
                printf("*****ERROR: command error*****\n");
                show_help();
                return;
            }
            else
            {
                while(*cmd == ' ')
                    cmd++;
            }
            if(cmd == NULL||cmd == '\0')
            {
                printf("*****ERROR: command error*****\n");
                return;
            }
            else
            {
                char * dr = "127.0.0.1";
                strncpy(dress_ftp,cmd,strlen(cmd));
                dress_ftp[strlen(cmd)] = '\0';
                printf("%s",dress_ftp);
                if(dress_ftp == "127.0.0.1")
                    if(1)
                    {
                        printf("Connect Seccessed!\n");
                        start_ftp_cmd(dr,DEFAULT_FTP_PORT);
                    }
                    else
                    {
                        printf("*****ERROR: Inviable Server Dress*****\n");
                    }
            }
        }
        else
        {
            switch(cmd_flag)
            {
                case 11:
                    local_list();
                    memset(usr_cmd,'\0',sizeof(usr_cmd));
                    break;
                case 12:
                    local_pwd();
                    memset(usr_cmd,'\0',sizeof(usr_cmd));
                    break;
                case 13:
                    local_cd(usr_cmd);
                    memset(usr_cmd,'\0',sizeof(usr_cmd));
                    break;
                case 6://quit
                    printf("BYE!\n");
                    printf("Thank you for use!\n");
                    exit(0);
                    break;
                default:
                    printf("*****ERROR: command error*****\n");
                    show_help();
                    memset(usr_cmd,'\0',sizeof(usr_cmd));
                    break;
            }
        }
    }

}


int ftp_usr_cmd(char * usr_cmd)
{
    if(!strncmp(usr_cmd,"open",4))
        return 15;
    if(!strncmp(usr_cmd,"close",5))
        return 16;
    if(!strncmp(usr_cmd,"mkdir",5))
        return 17;
    if(!strncmp(usr_cmd,"rmdir",5))
        return 18;
    if(!strncmp(usr_cmd,"dele",4))
        return 19;
    /* *************************************/
    if(!strncmp(usr_cmd,"ls",2))
        return 1;
    if(!strncmp(usr_cmd,"pwd",3))
        return 2;
    if(!strncmp(usr_cmd,"cd ",3))
        return 3;
    if(!strncmp(usr_cmd,"put ",4))
        return 4;
    if(!strncmp(usr_cmd,"get ",4))
        return 5;
    if(!strncmp(usr_cmd,"bye",3))
        return 6;
    if(!strncmp(usr_cmd,"mode",4))
        return 7;
    if(!strncmp(usr_cmd,"lls",3))
        return 11;
    if(!strncmp(usr_cmd,"lpwd",4))
        return 12;
    if(!strncmp(usr_cmd,"lcd ",4))
        return 13;
    return -1;
}




int start_ftp_cmd(char * host_ip_addr, int port) 
{
    int err;
    int cmd_flag;
    char usr_cmd[1024];
    err = fill_host_addr(host_ip_addr, &ftp_server, port);
    if(err == 254)
        cmd_err_exit("Invalid port!",254);
    if(err == 253)
        cmd_err_exit("Invalid server address!",253);

    sock_control = uconnect(&ftp_server,1);
    if((err =  ftp_get_reply(sock_control)) != 220)
        cmd_err_exit("Connect error!",220);
    do
    {
        err = ftp_login();
    }while(err != 1);

    while(1)
    {

        printf("ftp_client>");
        fgets(usr_cmd, 510, stdin);
        fflush(stdin);
        if(usr_cmd[0] == '\n')
            continue;
        usr_cmd[strlen(usr_cmd)-1] = '\0';
        cmd_flag = ftp_usr_cmd(usr_cmd);
        switch(cmd_flag)
        {
            case 1:
                ftp_list();
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 2:
                ftp_pwd();
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 3:
                ftp_cd(usr_cmd);
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 4:
                ftp_put(usr_cmd);
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 5:
                ftp_get(usr_cmd);
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 6:
                ftp_quit();
                printf("BYE!\n");
                printf("Thank you for use!\n");
                exit(0);
            case 7:
                ftp_mode = (ftp_mode + 1)%2;
                if(ftp_mode)//PASV
                    printf("change mode to PASV\n");
                else
                    printf("change mode to PORT\n");
                break;
            case 11:
                local_list();
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 12:
                local_pwd();
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
            case 13:
                local_cd(usr_cmd);
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;

            case 16:        
                close_cli();
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                open_srv();
                break;
            case 17:
                mkdir_srv(usr_cmd);
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
            case 18:
                rmdir_srv(usr_cmd);
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
            case 19:
                ftp_del(usr_cmd);
                memset(usr_cmd,'\0',sizeof(usr_cmd));
                break;
            default:
                show_help();
                memset(usr_cmd, '\0',sizeof(usr_cmd));
                break;
        }
    }
    return 1;
}

