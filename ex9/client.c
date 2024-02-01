#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>

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
    message[strlen(message)] = '\0';
    if (strstr(message, "OK") != NULL)
    {
        printf("OK\n");
    }
    else if (strstr(message, "goodbye") != NULL)
    {
        printf("Goodbye\n");
    }
    else
    {
        printf("%s\n", message);
    }
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

    struct pollfd fds[2];
    int timeout = -1;

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    while (1)
    {
        int activity = poll(fds, 2, timeout);

        if (activity == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN)
        {
            printf("username: ");
            send_message(client_buffer, sockfd);
            received_message(client_buffer, sockfd);

            if (strcmp(client_buffer, "Insert password") == 0)
            {
                printf("password: ");
                send_message(client_buffer, sockfd);
                received_message(client_buffer, sockfd);

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
        }

        if (fds[1].revents & POLLIN)
        {
            received_message(client_buffer, sockfd);
            printf("%s\n", client_buffer);
        }
    }

    close(sockfd);
    return 0;
}
