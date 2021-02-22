/*********************************************************************************
 *      Copyright:  (C) 2021 Lei Haojie
 *                  All rights reserved.
 *
 *       Filename:  ftp_client_cmd.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2021年02月13日)
 *         Author:  Lei Haojie <936700094@qq.com>
 *      ChangeLog:  1, Release initial version on "2021年02月13日 01时08分06秒"
 *                 
 ********************************************************************************/
#include "ftp_client.h"

int fill_host_addr(char * host_ip_addr, struct sockaddr_in * host, int port)
{
    if(port <= 0 || port > 65535) 
        return 254;
    //set the first sizeof(struce sockaddr_in) bits of host into zero
    bzero(host, sizeof(struct sockaddr_in));
    host->sin_family = AF_INET;
    //change the address from ip to in_addr
    if(inet_addr(host_ip_addr) != -1)
    {
        host->sin_addr.s_addr = inet_addr(host_ip_addr);
    }
    else 
    {
        if((server_hostent = gethostbyname(host_ip_addr)) != 0)
        {
            memcpy(&host->sin_addr, server_hostent->h_addr,sizeof(host->sin_addr));
        }
        else return 253;
    }
    host->sin_port = htons(port);
    return 1;
}


//display error message
void cmd_err_exit(char * err_msg, int err_code)
{
    printf("%s \n", err_msg);
    exit(err_code);
}

//    connect data stream
int uconnect_ftpdata()
{
    if(ftp_mode)//PASV
    {
        int server_data_port = get_port();
        if(server_data_port != 0)
            ftp_server.sin_port=htons(server_data_port);
        printf("Transfer in PASV mode\n");
        return(uconnect(&ftp_server, 0));
    }
    else// PORT
    {
        printf("Transfer in PORT mode\n");
        int client_port, get_sock, opt, temp;
        char cmd_buf[32];
        struct timeval outtime;
        struct sockaddr_in local;
        char local_ip[24];
        char *ip_1, *ip_2, *ip_3, *ip_4;
        int addr_len =  sizeof(struct sockaddr);
        client_port = rand_local_port();
        get_sock = socket(AF_INET, SOCK_STREAM, 0);
        if(get_sock < 0)
        {
            cmd_err_exit("socket()", 1);
        }

        // set outtime for the data socket
        outtime.tv_sec = 7;
        outtime.tv_usec = 0;
        opt = SO_REUSEADDR;
        temp = setsockopt(get_sock, SOL_SOCKET,SO_RCVTIMEO, &outtime,sizeof(outtime));
        if(temp !=0)
        {
            printf("*****ERROR: set socket %s errno:%d*****\n",strerror(errno),errno);
            cmd_err_exit("set socket", 1);
        }
        temp = setsockopt(get_sock, SOL_SOCKET,SO_REUSEADDR, &opt,sizeof(opt));
        if(temp !=0)
        {
            printf("*****ERROR: set socket %s errno:%d*****\n",strerror(errno),errno);
            cmd_err_exit("set socket", 1);
        }

        bzero(&local_host,sizeof(local_host));
        local_host.sin_family = AF_INET;
        local_host.sin_port = htons(client_port);
        local_host.sin_addr.s_addr = htonl(INADDR_ANY);
        bzero(&local, sizeof(struct sockaddr));
        while(1)
        {
            temp = bind(get_sock, (struct sockaddr *)&local_host, sizeof(local_host));
            if(temp != 0 && errno == 11)
            {
                client_port = rand_local_port();
                continue;
            }
            temp = listen(get_sock, 1);
            if(temp != 0 && errno == 11)
            {
                cmd_err_exit("listen()", 1);
            }
            //   get local host's ip
            if(getsockname(sock_control,(struct sockaddr*)&local,(socklen_t *)&addr_len) < 0)
                return -1;
            snprintf(local_ip, sizeof(local_ip), inet_ntoa(local.sin_addr));
            // change the format to the PORT command needs
            local_ip[strlen(local_ip)]='\0';
            ip_1 = local_ip;
            ip_2 = strchr(local_ip, '.');
            *ip_2 = '\0';
            ip_2++;
            ip_3 = strchr(ip_2, '.');
            *ip_3 = '\0';
            ip_3++;
            ip_4 = strchr(ip_3, '.');
            *ip_4 = '\0';
            ip_4++;
            snprintf(cmd_buf, sizeof(cmd_buf), "PORT %s,%s,%s,%s,%d,%d", ip_1, ip_2, ip_3, ip_4, client_port >> 8, client_port&0xff);
            ftp_send_cmd(cmd_buf, NULL, sock_control);
            if(ftp_get_reply(sock_control) != 200)
            {
                printf("Cannot use PORT mode!Please use \"mode\" change to PASV mode.\n");
                return -1;
            }
            else
                return get_sock;
        }
    }
}




/*get user and password for login*/
void get_username()
{
    char read_buf[64];
    printf("Please input user name(Press enter for anonymous): ");
    fgets(read_buf, sizeof(read_buf), stdin);
    if(read_buf[0]=='\n')
        strncpy(user, "anonymous", 9);
    else
        strncpy(user, read_buf, strlen(read_buf)-1);
}


void get_passwd()
{
    char read_buf[64];
    printf("Please input password(Press enter for anonymous): ");
    fgets(read_buf, sizeof(read_buf), stdin);
    if(read_buf[0]=='\n')
        strncpy(passwd, "anonymous", 9);
    else
        strncpy(passwd, read_buf, strlen(read_buf)-1);
    //display the information
    printf("\n");
}



/* connect to ftp server*/
int uconnect(struct sockaddr_in *s_addr, int type)
{
    struct timeval outtime;
    int set;
    int s1 = socket(AF_INET, SOCK_STREAM, 0);//create socket
    if(s1 < 0)
        cmd_err_exit("*****ERROR: creat socket error*****", 249);

    // set outtime for the control socket
    if(type == 1)
    {
        outtime.tv_sec = 0;
        outtime.tv_usec = 300000;
    }
    else
    {
        outtime.tv_sec = 10;
        outtime.tv_usec = 0;
    }
    set = setsockopt(s1, SOL_SOCKET,SO_RCVTIMEO, &outtime,sizeof(outtime));
    if(set != 0)
    {
        printf("*****ERROR: set socket %s errno:%d******\n",strerror(errno),errno);
        cmd_err_exit("set socket", 1);
    }

    //  connect to the server
    if (connect(s1,(struct sockaddr *)s_addr,sizeof(struct sockaddr_in)) < 0)
    {
        printf("*****ERROR: cannot connect to server %s, port %d*****\n",inet_ntoa(s_addr->sin_addr),ntohs(ftp_server.sin_port));
        exit(252);
    }
    return s1;
}



int get_port()
{
    char port_respond[512];
    char *buf_ptr;
    int count,port_num;
    ftp_send_cmd("PASV",NULL,sock_control);
    count = read(sock_control, port_respond, 510);
    if(count <= 0)
        return 0;
    port_respond[count]='\0';
    if(atoi(port_respond) == 227)
    {
        //get low byte of the port
        buf_ptr = strrchr(port_respond, ',');
        port_num = atoi(buf_ptr + 1);
        *buf_ptr = '\0';
        //get high byte of the port
        buf_ptr = strrchr(port_respond, ',');
        port_num += atoi(buf_ptr + 1) * 256;
        return port_num;
    }
    return 0;
}

int rand_local_port()
{
    int local_port;
    srand((unsigned)time(NULL));
    local_port = rand() % 40000 + 1025;
    return local_port;
}


int ftp_login()
{
    int info;
    get_username();
    if(ftp_send_cmd("USER ", user, sock_control) < 0)
        cmd_err_exit("*****ERROR: cannot send message*****",1);
    info = ftp_get_reply(sock_control);
    if(info == 331)//user logged in, password needed
    {
        get_passwd();
        if(ftp_send_cmd("PASS ", passwd, sock_control) <= 0)
            cmd_err_exit("*****ERROR: cannot send message*****",1);
        else
            info = ftp_get_reply(sock_control);
        if(info == 230)//user log in
            return 1;
        else if(info == 531)//anonymous user
            return 1;
        else
        {
            printf("*****ERROR: password error*****\n");
            return 0;
        }

    }
    else
    {
        printf("******ERROR: User name error*****\n");
        return 0;
    }
}





/*Function to display the files in the current directory of the server.*/

void ftp_list()
{   
    int i = 0,new_sock;
    int set = sizeof(local_host);
    int list_sock_data = uconnect_ftpdata();
    if(list_sock_data < 0)
    {
        ftp_get_reply(sock_control);
        printf("creat data sock error!\n");
        return;
    }
    ftp_get_reply(sock_control);
    ftp_send_cmd("LIST", NULL, sock_control);
    ftp_get_reply(sock_control);
    if(ftp_mode)//PASV
        ftp_get_reply(list_sock_data);
    else
    {
        while(i < 3)
        {
            new_sock = accept(list_sock_data, (struct sockaddr *)&local_host, (socklen_t *)&set);
            if(new_sock == -1)
            {
                printf("accept return:%s errno: %d\n", strerror(errno),errno);
                i++;
                continue;
            }
            else break;
        }
        if(new_sock == -1)
        {
            printf("Sorry, you can't use PORT mode. There is something wrong when the server connect to you.\n");
            return;
        }
        ftp_get_reply(new_sock);
        close(new_sock);
    }

    close(list_sock_data);
    ftp_get_reply(sock_control);
}



/*此函数读取客户端命令后的文件名字。在很多命令中，如get，put后要加入上传或下载的文件名字，这个函数解析文件名字信息*/
void ftp_cmd_filename(char * usr_cmd, char * src_file, char * dst_file)
{   
    int length,  flag = 0;
    int i = 0, j = 0;
    char * cmd_src;
    cmd_src = strchr(usr_cmd, ' ');
    if(cmd_src == NULL)
    {
        printf("command error!\n");
        return;
    }
    else
    {
        while(*cmd_src == ' ')
            cmd_src ++;
    }
    if(cmd_src == NULL || cmd_src == '\0')
    {
        printf("command error!\n");
    }
    else
    {   
        length = strlen(cmd_src);
        while(i <= length)//be careful with space in the filename
        {   
            if((*(cmd_src+i)) !=' ' && (*(cmd_src+i)) != '\\')
            {
                if(flag == 0)
                    src_file[j] = *(cmd_src +i);
                else
                    dst_file[j] = *(cmd_src +i);
                j++;
            }
            if((*(cmd_src+i)) == '\\' && (*(cmd_src+i+1)) !=' ')
            {
                if(flag == 0)
                    src_file[j] = *(cmd_src +i);
                else
                    dst_file[j] = *(cmd_src +i);
                j++;
            }
            if((*(cmd_src+i)) == ' ' && (*(cmd_src+i-1)) != '\\')
            {
                src_file[j] = '\0';
                flag = 1;
                j = 0;
            }
            if((*(cmd_src+i)) == '\\' && (*(cmd_src+i+1)) == ' ')
            {
                if(flag == 0)
                    src_file[j] = ' ';
                else
                    dst_file[j] = ' ';
                j++;
            }
            i++;
        };
    }
    if(flag == 0)
        strcpy(dst_file, src_file);
    else
        dst_file[j] = '\0';
}


/*download file*/
void ftp_get(char * usr_cmd)
{
    int get_sock, set, new_sock, i = 0;
    char src_file[512];
    char dst_file[512];
    char rcv_buf[512];
    char cover_flag[3];
    struct stat file_info;
    int local_file;
    int count = 0;
    ftp_cmd_filename(usr_cmd, src_file, dst_file);
    ftp_send_cmd("SIZE ", src_file, sock_control);
    if(ftp_get_reply(sock_control) != 213)
    {
        printf("SIZE error!\n");
        return;
    }
    if(!stat(dst_file, &file_info))
    {
        printf("local file %s exists: %d bytes\n", dst_file, (int)file_info.st_size);
        printf("Do you want to cover it or contiue to transfer? Press 'y' for cover, 'n' for contiue[y/n]");
        fgets(cover_flag, sizeof(cover_flag), stdin);
        fflush(stdin);
        if(cover_flag[0] != 'y')
        {
            printf("get file %s aborted.\n", src_file);
            return;
        }
        else    printf("Sorry,the code is not finished as time limited.");
    }

    local_file = open(dst_file, O_CREAT|O_TRUNC|O_WRONLY);
    if(local_file < 0)
    {
        printf("*****ERROR: creat local file %s error*****\n", dst_file);
        return;
    }
    get_sock = uconnect_ftpdata();
    if(get_sock < 0)
    {
        printf("*****ERROR: socket error*****\n");
        return;
    }
    set = sizeof(local_host);

    ftp_send_cmd("TYPE I", NULL, sock_control);
    ftp_get_reply(sock_control);
    ftp_send_cmd("RETR ", src_file, sock_control);
    if(!ftp_mode)//PORT
    {
        while(i < 3)
        {
            new_sock = accept(get_sock, (struct sockaddr *)&local_host, (socklen_t *)&set);
            if(new_sock == -1)
            {
                printf("*****ERROR: accept return:%s errno: %d*****\n", strerror(errno),errno);
                i++;
                continue;
            }
            else break;
        }
        if(new_sock == -1)
        {
            printf("Sorry, you can't use PORT mode. There is something wrong when the server connect to you.\n");
            return;
        }
        ftp_get_reply(sock_control);
        while(1)
        {
            printf("loop \n");
            count = read(new_sock, rcv_buf, sizeof(rcv_buf));
            if(count <= 0)
                break;
            else
            {
                write(local_file, rcv_buf, count);
            }
        }
        close(local_file);
        close(get_sock);
        close(new_sock);
        ftp_get_reply(sock_control); 
    }
    else
    {
        ftp_get_reply(sock_control);
        while(1)
        {
            count = read(get_sock, rcv_buf, sizeof(rcv_buf));
            if(count <= 0)
                break;
            else
            {
                write(local_file, rcv_buf, count);
            }
        }
        close(local_file);
        close(get_sock);
        ftp_get_reply(sock_control); 
    }
    if(!chmod(src_file, 0644))
    {
        printf("chmod %s to 0644\n", dst_file);
        return;
    }
    else
        printf("*****ERROR: chmod %s to 0644 error*****\n", dst_file);
    ftp_get_reply(sock_control); 
}


/*put file*/
void ftp_put(char * usr_cmd)
{   
    char src_file[512];
    char dst_file[512];
    char send_buf[512];
    struct stat file_info;
    int local_file;
    int file_put_sock, new_sock, count = 0, i = 0;
    int set = sizeof(local_host);
    ftp_cmd_filename(usr_cmd, src_file, dst_file);
    if(stat(src_file, &file_info) < 0)
    {
        printf("*****ERROR: local file %s doesn't exist*****\n", src_file);
        return;
    }
    local_file = open(src_file, O_RDONLY);
    if(local_file < 0)
    {
        printf("*****ERROR: open local file %s error*****\n", dst_file);
        return;
    }
    file_put_sock = uconnect_ftpdata();
    if(file_put_sock < 0)
    {   
        ftp_get_reply(sock_control);
        printf("*****ERROR: creat data sock errro*****\n");
        return;
    }
    ftp_send_cmd("STOR ", dst_file, sock_control);
    ftp_get_reply(sock_control);
    ftp_send_cmd("TYPE I", NULL, sock_control);
    ftp_get_reply(sock_control);
    if(!ftp_mode)//PROT
    {
        while(i < 3)
        {
            new_sock = accept(file_put_sock, (struct sockaddr *)&local_host, \
                    (socklen_t *)&set);
            if(new_sock == -1)
            {
                printf("*****ERROR: accept return:%s errno: %d*****\n", strerror(errno),errno);
                i++;
                continue;
            }
            else break;
        }
        if(new_sock == -1)
        {
            printf("Sorry, you can't use PORT mode. There is something wrong when the server connect to you.\n");
            return;
        }
        while(1)
        {
            count = read(local_file, send_buf, sizeof(send_buf));
            if(count <= 0)
                break;
            else
            {
                write(new_sock, send_buf,sizeof(send_buf));
            }
        }
        close(local_file);
        close(file_put_sock);
        close(new_sock);
    }
    else
    {
        while(1)
        {
            count = read(local_file, send_buf, sizeof(send_buf));
            if(count <= 0)
                break;
            else
            {
                write(file_put_sock, send_buf,count);
            }
        }
        close(local_file);
        close(file_put_sock);
    }
    ftp_get_reply(sock_control); 
}


/*quit*/
void ftp_quit()
{
    ftp_send_cmd("QUIT",NULL,sock_control);
    ftp_get_reply(sock_control);
    close(sock_control);
}


/*close*/
void close_cli()
{
    ftp_send_cmd("CLOSE",NULL,sock_control);
    ftp_get_reply(sock_control);
}


/*pwd*/
void ftp_pwd()
{
    ftp_send_cmd("PWD", NULL, sock_control);
    ftp_get_reply(sock_control);
}

/*cd*/
void ftp_cd(char * usr_cmd)
{
    char *cmd = strchr(usr_cmd, ' ');
    char path[1024];
    if(cmd == NULL)
    {
        printf("*****ERROR: cd command error*****\n");
        return;
    }
    else
    {
        while(*cmd == ' ')
            cmd ++;
    }
    if(cmd == NULL || cmd == '\0')
    {
        printf("*****ERROR: cd command error*****\n");
        return;
    }
    else
    {
        strncpy(path, cmd, strlen(cmd));
        path[strlen(cmd)]='\0';
        ftp_send_cmd("CWD ", path, sock_control);
        ftp_get_reply(sock_control);
    }
}


/*delete file*/
void ftp_del(char *usr_cmd)
{
    char *cmd = strchr(usr_cmd,' ');
    char filename[1024];
    if(cmd == NULL)
    {
        printf("*****ERROR: delete command error*****\n");
        return;
    }
    else
    {
        while(*cmd == ' ')
            cmd++;
    }
    if(cmd == NULL||cmd =='\0')
    {
        printf("*****ERROR: delete command error*****\n");
        return;
    }
    else
    {
        strncpy(filename,cmd,strlen(cmd));
        filename[strlen(cmd)]='\0';
        ftp_send_cmd("DELE",filename,sock_control);
        ftp_get_reply(sock_control);
    }
}


/*mkdir*/
void mkdir_srv(char *usr_cmd)
{
    char *cmd = strchr(usr_cmd,' ');
    char path[1024];
    if(cmd == NULL)
    {
        printf("*****ERROR: mkdir command error*****\n");
        return;
    }
    else
    {
        while(*cmd == ' ')
            cmd ++;
    }
    if(cmd == NULL||cmd == '\0')
    {
        printf("*****ERROR: mkdir command error*****\n");
        return;
    }
    else
    {
        strncpy(path,cmd,strlen(cmd));
        path[strlen(cmd)]='\0';
        ftp_send_cmd("MKD",path,sock_control);
        ftp_get_reply(sock_control);
    }
}



/*rm -rf*/
void rmdir_srv(char *usr_cmd)
{
    char *cmd = strchr(usr_cmd,' ');
    char path[1024];
    if(cmd == NULL)
    {
        printf("*****ERROR: rmdir command error*****\n");
        return;
    }
    else
    {
        while(*cmd == ' ')
            cmd++;
    }
    if(cmd == NULL||cmd == '\0')
    {
        printf("*****ERROR: rmdir command error*****\n");
        return;
    }
    else
    {
        strncpy(path,cmd,strlen(cmd));
        path[strlen(cmd)]='\0';
        ftp_send_cmd("RMD",path,sock_control);
        ftp_get_reply(sock_control);
    }
}


/*List client files*/
void local_list()
{
    DIR * dp;
    struct dirent *dirp;
    if((dp = opendir("./")) == NULL)
    {
        printf("*****ERROR: opendir() error*****\n");
        return;
    }
    printf("Local file list:\n");
    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;
        printf("%s\n", dirp->d_name);
    }
}


/*pwd*/
void local_pwd()
{
    char curr_dir[512];
    int size = sizeof(curr_dir);
    if(getcwd(curr_dir, size) == NULL)
        printf("*****ERROR: getcwd failed*****\n");
    else
        printf("Current local directory: %s\n", curr_dir);
}




/*cd*/
void local_cd(char * usr_cmd)
{
    char *cmd = strchr(usr_cmd, ' ');
    char path[1024];
    if(cmd == NULL)
    {
        printf("*****ERROR: local cd command error*****\n");
        return;
    }
    else
    {
        while(*cmd == ' ')
            cmd ++;
    }
    if(cmd == NULL || cmd == '\0')
    {
        printf("*****ERROR: local cd command error*****\n");
        return;
    }
    else
    {
        strncpy(path, cmd, strlen(cmd));
        path[strlen(cmd)]='\0';
        if(chdir(path) < 0)
            printf("*****ERROR: Local: chdir to %s error*****\n", path);
        else
            printf("Local: chdir to %s\n", path);
    }
}


void show_help()
{   
    printf("\033[32mhelp\033[0m\t--print this command list\n");
    printf("\033[32mopen\033[0m\t--open the server\n");
    printf("\033[32mclose\033[0m\t--close the connection with the server\n");
    printf("\033[32mmkdir\033[0m\t--make new dir on the ftp server\n");
    printf("\033[32mrmdir\033[0m\t--delete the dir on the ftp server\n");
    printf("\033[32mpwd\033[0m\t--print the current directory of server\n");
    printf("\033[32mls\033[0m\t--list the files and directoris in current directory of server\n");
    printf("\033[32mcd [directory]\033[0m\n\t--enter  of server\n");
    printf("\033[32mmode\033[0m\n\t--change current mode, PORT or PASV\n");
    printf("\033[32mput [local_file] \033[0m\n\t--send [local_file] to server as \n");
    printf("\tif  isn't given, it will be the same with [local_file] \n");
    printf("\tif there is any \' \' in , write like this \'\\ \'\n");
    printf("\033[32mget [remote file] \033[0m\n\t--get [remote file] to local host as\n");
    printf("\tif  isn't given, it will be the same with [remote_file] \n");
    printf("\tif there is any \' \' in , write like this \'\\ \'\n");
    printf("\033[32mlpwd\033[0m\t--print the current directory of local host\n");
    printf("\033[32mlls\033[0m\t--list the files and directoris in current directory of local host\n");
    printf("\033[32mlcd [directory]\033[0m\n\t--enter  of localhost\n");
    printf("\033[32mbye\033[0m\t--quit this ftp client program\n");
}
