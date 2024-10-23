#include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/wait.h>
    #include <utmp.h>
    #include <time.h>

    #define CLIENT_SERVER "client_to_server"
    #define SERVER_CLIENT "server_client"

    int main()
    {
        unlink(SERVER_CLIENT);
        unlink(CLIENT_SERVER);
        int client_server,server_client;
      
        if(mknod(CLIENT_SERVER, S_IFIFO | 0666, 0)==-1)
        {
            perror("Eroare la mknod");
            exit(0);
        }
        if(mknod(SERVER_CLIENT, S_IFIFO | 0666, 0)==-1)
        {
            perror("Eroare la mknod");
            exit(0);
        }

        printf("Astept sa intre cineva\n");
        if((client_server = open(CLIENT_SERVER, O_RDONLY))==-1)
        {
            perror("Eroare la open");
            exit(1);
        }
        if((server_client = open(SERVER_CLIENT,O_WRONLY))==-1)
        {
            perror("Eroare la open");
            exit(1);
        }
        printf("A intrat cineva pe server:\n");
        int bytes_in,quit=1,login=0;
        char buffer[1024];
        while(quit==1)
        {
            bytes_in=read(client_server,buffer,51);
            buffer[bytes_in]='\0';
            printf("S-a citit:%s\n",buffer);
            if(strstr(buffer,"login : ")&&login==0) ///PIPE
            {
                int pid,p[2];
                char raspuns[3];
                if(-1==pipe(p))
                {
                    perror("pipe");
                    exit(2);
                }
                if((pid=fork())==-1)
                {
                    perror("Fork");
                    exit(3);
                }
                if(pid)///parinte
                {
                    write(p[1],buffer,strlen(buffer));
                    close(p[1]);
                    wait(NULL);
                    read(p[0],raspuns,sizeof(raspuns));
                    close(p[0]);
                    if(strcmp(raspuns,"Da"))
                    {
                        strcpy(buffer,"Te-ai conectat cu succes");
                        login=1;
                    }
                    else
                        strcpy(buffer,"Nu exista acest username");
                }
                else///copil
                {
                    read(p[0],buffer,1024);
                    close(p[0]);
                    int file_usernames,ok=0;
                    if((file_usernames=open("usernames.txt",O_RDONLY))==-1)
                    {
                        perror("[copil]open");
                        exit(7);
                    }
                    char *nume=strtok(buffer," ");
                    for(int i=1;i<=2;i++)
                        nume=strtok(NULL," ");
                    char numefisier[256];
                    if((read(file_usernames,numefisier,256))==-1)
                    {
                        perror("[copil]read");
                        exit(8);
                    }
                    char *p=strtok(numefisier,"/");
                    while(p!=NULL)
                    {
                        if(strcmp(p,nume)==0)
                        {
                            ok=1;
                            break;
                        }
                        p=strtok(NULL,"/");
                    }
                    if(ok==1)
                    strcpy(raspuns,"Da");
                    else
                    strcpy(raspuns,"Nu");
                    write(p[1],raspuns,strlen(raspuns));
                    close(p[1]);
                    exit(0);
                }
            }
            else
            if(strcmp(buffer,"quit")==0&&login==0) ///SOCKET
            {
                int pid,sockp[2];
                quit=0;
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                { 
                    perror("socketpair"); 
                    exit(2); 
                }
                if((pid=fork())==-1)
                {
                    perror("Fork");
                    exit(3);
                }
                if(pid)///parinte
                {
                    close(sockp[0]); 
                    if (write(sockp[1], buffer, strlen(buffer)) < 0) 
                    {
                        perror("[parinte]write"); 
                        exit(4);
                    }
                    if (read(sockp[1], buffer, 1024) < 0) 
                    {
                        perror("[parinte]read"); 
                        exit(5);
                    }
                    close(sockp[1]); 
                }
                else///copil
                {
                    close(sockp[1]);
                    if (read(sockp[0], buffer, 1024) < 0)
                    {
                        perror("[copil]read");
                        exit(6);
                    }
                    strcpy(buffer,"QUIT");
                    if (write(sockp[0], buffer, strlen(buffer)) < 0)
                    {
                        perror("[copil]write"); 
                    }
                    close(sockp[0]);
                    exit(1);
                }
            }
            else
            if(strcmp(buffer,"loggout")==0&&login==1) ///SOCKET
            {
                int pid,sockp[2];
                login=0;
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                { 
                    perror("socketpair"); 
                    exit(2); 
                }
                if((pid=fork())==-1)
                {
                    perror("Fork");
                    exit(3);
                }
                if(pid)///parinte
                {
                    close(sockp[0]); 
                    if (write(sockp[1], buffer, strlen(buffer)) < 0) 
                    {
                        perror("[parinte]write"); 
                        exit(4);
                    }
                    if (read(sockp[1], buffer, 1024) < 0) 
                    {
                        perror("[parinte]read"); 
                        exit(5);
                    }
                    close(sockp[1]); 
                }
                else///copil
                {
                    close(sockp[1]);
                    if (read(sockp[0], buffer, 1024) < 0)
                    {
                        perror("[copil]read");
                        exit(6);
                    }
                    strcpy(buffer,"Te-ai deconectat de la cont");
                    if (write(sockp[0], buffer, strlen(buffer)) < 0)
                    {
                        perror("[copil]write"); 
                    }
                    close(sockp[0]);
                    exit(1);
                }
            }
            else
            if(strcmp(buffer,"get-logged-users")==0&&login==1)
            {
                int pid,sockp[2];
                memset(buffer,0,sizeof(buffer));
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                { 
                    perror("socketpair"); 
                    exit(2); 
                }
                if((pid=fork())==-1)
                {
                    perror("Fork");
                    exit(3);
                }
                if(pid)///parinte
                {
                     close(sockp[0]);
                    if (read(sockp[1], buffer, 1024) < 0) 
                    {
                        perror("[parinte]read"); 
                        exit(5);
                    }
                    close(sockp[1]); 
                }
                else///copil
                {
                    close(sockp[1]);
                    struct utmp* info;
                    char user[1024]="";
                    while((info=getutent())!=NULL)
                    {
                        if(info->ut_type==USER_PROCESS)
                        {
                            strncat(user,info->ut_user,UT_NAMESIZE);
                            strcat(user," ");
                            strncat(user,info->ut_host,UT_HOSTSIZE);
                            strcat(user," ");
                            time_t time=info->ut_tv.tv_sec;
                            strcat(user,ctime(&time));
                        }
                    }
                     if (write(sockp[0], user, strlen(user)) < 0) perror("[copil]Err...write"); 
                    close(sockp[0]);
                    endutent();
                    exit(1); 
                }
            }
            else
            if(strncmp(buffer,"get-proc-info : ",16)==0&&login==1)
            {
                 int pid,sockp[2];
                if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                { 
                    perror("socketpair"); 
                    exit(2); 
                }
                if((pid=fork())==-1)
                {
                    perror("Fork");
                    exit(3);
                }
                if(pid)///parinte
                {
                    close(sockp[0]); 
                    if (write(sockp[1], buffer, strlen(buffer)) < 0) 
                    {
                        perror("[parinte]write"); 
                        exit(4);
                    }
                    memset(buffer,0,strlen(buffer));
                    if (read(sockp[1], buffer, 1024) < 0) 
                    {
                        perror("[parinte]read"); 
                        exit(5);
                    }
                    close(sockp[1]); 
                }
                else///copil
                {
                    close(sockp[1]);
                    FILE* input;
                    char com[1024];
                    char ras[1024];
                    if (read(sockp[0], buffer, 1024) < 0)
                    {
                        perror("[copil]read");
                        exit(6);
                    }
                    char*p=strtok(buffer," ");
                    for(int i=1;i<=2;i++)
                        p=strtok(NULL," ");
                    memset(com,0,sizeof(com));
                    strcat(com,"/proc/");
                    strcat(com,p);
                    strcat(com,"/status");
                    printf("%s\n",com);
                    if((input=fopen(com,"r"))==NULL)
                    {
                        memset(buffer,0,sizeof(buffer));
                        strcpy(buffer,"Eroare la executia comenzii");
                        write(sockp[0], buffer, strlen(buffer));
                        close(sockp[0]);
                        exit(1);
                    }
                    memset(buffer,0,sizeof(buffer));
                    while(fgets(ras,sizeof(ras),input)!=NULL)
                    {
                        if(strncmp(ras,"Name:",5)==0)
                        {
                            strcat(buffer,"(");
                            strcat(buffer,ras+6);
                            buffer[strlen(buffer)-1]='\0';
                            strcat(buffer,")");
                        }
                        if(strncmp(ras,"State:",6)==0)
                        {
                            strcat(buffer,"(");
                            strcat(buffer,ras+7);
                            buffer[strlen(buffer)-1]='\0';
                            strcat(buffer,")");
                        }
                        if(strncmp(ras,"PPid:",5)==0)
                        {
                            strcat(buffer,"(");
                            strcat(buffer,ras+6);
                            buffer[strlen(buffer)-1]='\0';
                            strcat(buffer,")");
                        }
                        if(strncmp(ras,"Uid:",4)==0)
                        {
                            strcat(buffer,"(");
                            strcat(buffer,ras+5);
                            buffer[strlen(buffer)-1]='\0';
                            strcat(buffer,")");
                        }
                        if(strncmp(ras,"VmSize:",7)==0)
                        {
                            strcat(buffer,"(");
                            strcat(buffer,ras+8);
                            buffer[strlen(buffer)-1]='\0';
                            strcat(buffer,")");
                        }
                    }
                    if (write(sockp[0], buffer, strlen(buffer)) < 0)
                    {
                        perror("[copil]write"); 
                    }
                    close(sockp[0]);
                    exit(1);
                }
            }
            else
            {
                memset(buffer,0,sizeof(buffer));
                strcpy(buffer,"Comanda nu exista");
            }
            write(server_client,buffer,strlen(buffer));
            memset(buffer,0,sizeof(buffer));
        }
        printf("Am inchis si serverul\n");
        unlink(SERVER_CLIENT);
        unlink(CLIENT_SERVER);
    }