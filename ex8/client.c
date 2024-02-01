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
    send(sockfd, message, strlen(message), 0);
}

void received_message(char *message, int sockfd)
{
    recv(sockfd, message, MESSAGE_SIZE, 0);
    printf("%ld\n", strlen(message));
    message[strlen(message)] = '\0';
    printf("%s\n", message);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of parameters!\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    char client_buffer[MESSAGE_SIZE];
    struct sockaddr_in server_addr;
    int port = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Can not connect to server! Client exits immediately! ");
        return 0;
    }
    fd_set read_fds;
    int max_fd = sockfd + 1;
    while (1)
    {

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);

        select(max_fd, &read_fds, NULL, NULL, NULL);
        // if (FD_ISSET(STDIN_FILENO, &read_fds))
        // {
        printf("username: ");
        send_message(client_buffer, sockfd);
        received_message(client_buffer, sockfd);
        printf("%s\n", client_buffer);

        if (strcmp(client_buffer, "Insert password") == 0)
        {
            printf("password: ");
            send_message(client_buffer, sockfd);
            received_message(client_buffer, sockfd);
            if (strstr(client_buffer, "OK") != NULL)
            {
                printf("OK\n");
            }
            else if (strstr(client_buffer, "goodbye") != NULL)
            {
                printf("Goodbye\n");
            }
            
            else
            {
                printf("%s\n", client_buffer);
            }

            if (strstr(client_buffer, "OK") != NULL)
            {
                do
                {
                    printf("reset password: ");
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
                    else if (strstr(response, "goodbye") != NULL)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
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
        // }

        if (FD_ISSET(sockfd, &read_fds))
        {
            // Server response
            received_message(client_buffer, sockfd);
            printf("%s\n", client_buffer);
        }
    }

    close(sockfd);
    return 0;
}
