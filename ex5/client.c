#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MESSAGE_SIZE 1024

void send_message(char *message, int sockfd)
{
    memset(message, 0, MESSAGE_SIZE);
    fgets(message, MESSAGE_SIZE, stdin);

    message[strlen(message) - 1] = '\0';
    send(sockfd, message, strlen(message),0);
}

void received_message(char *message, int sockfd)
{
    recv(sockfd, message, MESSAGE_SIZE,0);
    message[strlen(message)] = '\0';
}

int main(int argc, char *argv[])
{
    int sockfd;
    char client_buffer[MESSAGE_SIZE];
    struct sockaddr_in server_addr;
    int port = atoi(argv[2]);
    int received_mess;
    if (argc != 3)
    {
        printf("Wrong number of parameters!\n");
        exit(EXIT_FAILURE);
    }
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

    while (1)
    {
        printf("username : ");

        send_message(client_buffer, sockfd);
        received_message(client_buffer, sockfd);
        printf("%s\n", client_buffer);
        if (strcmp(client_buffer, "Insert password") == 0)
        {
            printf("password : ");
            send_message(client_buffer, sockfd);
            received_message(client_buffer, sockfd);
            printf("%s\n", client_buffer);

            if (strcmp(client_buffer, "OKa") == 0)
            {
                printf("vao");
                do
                {
                    printf("resert password : ");
                    send_message(client_buffer, sockfd);
                    char response[1024];
                    received_message(response, sockfd);
                    printf("%s\n", response);

                    if (strcmp(response, client_buffer) == 0)
                    {
                        continue;
                    }
                    else if (strcmp(response, "Error") == 0)
                    {
                        continue;
                    }
                    else
                        break;
                } while (1);
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }
    close(sockfd);
    return 0;
}
