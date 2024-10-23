#include <stdio.h>
    #include <stdlib.h>
    #include <errno.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>

    #define CLIENT_SERVER "client_to_server"
        #define SERVER_CLIENT "server_client"

    int main()
    {
        int client_server,server_client;

        if(-1==(client_server = open(CLIENT_SERVER, O_WRONLY)))
        {
            perror("Nu e deschis serverul");
            exit(0);
        }
        if(-1==(server_client = open(SERVER_CLIENT,O_RDONLY)))
        {
            perror("Eroare la open");
            exit(0);
        }
        printf("Am intrat pe server\n");
        printf("Poti tasta comenzi de aici\n");
        int bytes_in,ok=1;
        char buffer[1024];
        while (gets(buffer), !feof(stdin)&&ok==1)
        {
            bytes_in=write(client_server,buffer,strlen(buffer));
            memset(buffer,0,sizeof(buffer));
            read(server_client,buffer,1024);
            printf("Am primit raspunsul de la server:[%d] %s\n",strlen(buffer),buffer);
            if(strcmp(buffer,"QUIT")==0)
            {
                ok=0;
                printf("Iesim de pe server\n");
                break;
            }
            memset(buffer,0,sizeof(buffer));
        }
        printf("Am iesit\n");
        return 0;
    }
