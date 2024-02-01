#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MESSAGE_SIZE 1024

void send_message(char *message, int sockfd, struct sockaddr_in server_addr)
{
    memset(message, 0, MESSAGE_SIZE);
    fgets(message, MESSAGE_SIZE, stdin);

    message[strlen(message) - 1] = '\0';
    sendto(sockfd, message, strlen(message),
           MSG_CONFIRM, (const struct sockaddr *)&server_addr,
           sizeof(server_addr));
}

void received_message(int received_mess, char *message, int sockfd, struct sockaddr_in server_addr)
{
    socklen_t addr_size = sizeof(server_addr);
    received_mess = recvfrom(sockfd, message, MESSAGE_SIZE,
                             MSG_WAITALL, (struct sockaddr *)&server_addr,
                             &addr_size);
    message[received_mess] = '\0';
}

int main(int argc, char *argv[])
{
    int sockfd;
    char client_buffer[MESSAGE_SIZE];
    struct sockaddr_in server_addr;
    int received_mess;
    if (argc != 3)
    {
        printf("Wrong number of parameters!\n");
        exit(EXIT_FAILURE);
    }
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("--Socket created\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    while (1)
    {
        printf("username : ");

        send_message(client_buffer, sockfd, server_addr);
        received_message(received_mess, client_buffer, sockfd, server_addr);
        printf("%s\n", client_buffer);
        if (strcmp(client_buffer, "Insert password") == 0)
        {
            printf("password : ");
            send_message(client_buffer, sockfd, server_addr);
            received_message(received_mess, client_buffer, sockfd, server_addr);
            printf("%s\n", client_buffer);

            if (strcmp(client_buffer, "OK") == 0)
            {
                do
                {
                    printf("resert password : ");
                    send_message(client_buffer, sockfd, server_addr);
                    char response[1024];
                    received_message(received_mess, response, sockfd, server_addr);
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
