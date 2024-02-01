#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <ctype.h>

#define MESSAGE_SIZE 1024
#define BLOCK_THRESHOLD 3
#define MAX_CLIENTS 10

enum AccountStatus
{
    ACTIVE = 1,
    BLOCKED = 0,
    IDLE = 2
};

int checkPassword = 0;

typedef struct User
{
    char username[100];
    char password[100];
    int status;
} User;

typedef struct Node
{
    User user;
    int isOnline;
    struct Node *pNext;
} Node;

Node *push(Node *root, User user)
{
    Node *new = (Node *)malloc(sizeof(Node));
    new->user = user;
    new->pNext = NULL;
    if (root == NULL)
    {
        root = new;
    }
    else
    {
        new->pNext = root;
        root = new;
    }

    return root;
}

Node *readData(Node *root)
{
    FILE *file;
    User temp;
    file = fopen("account.txt", "r");
    if (file != NULL)
    {
        while (fscanf(file, "%s %s %d", temp.username, temp.password, &temp.status) != EOF)
        {
            root = push(root, temp);
        }
    }
    fclose(file);
    return root;
}

void saveData(Node *root)
{
    FILE *file;
    file = fopen("account.txt", "wt");
    while (root != NULL)
    {
        fprintf(file, "%s %s %d\n", root->user.username, root->user.password, root->user.status);
        root = root->pNext;
    }
    fclose(file);
}

void emptyList(Node *first)
{
    Node *tmp;
    while (first != NULL)
    {
        tmp = first;
        first = first->pNext;
        free(tmp);
    }
}

void send_message(char message[], int sockfd)
{
    send(sockfd, message, strlen(message), 0);
}

Node *login(Node *root, int sockfd, struct sockaddr_in client_addr)
{
    char username[MESSAGE_SIZE], password[MESSAGE_SIZE];
    int checkUsername = 0, checkPassword = 0;
    Node *currentUser = root;
    printf("__________Login__________\n");

    do
    {
        printf("Username: ");
        recv(sockfd, username, MESSAGE_SIZE, 0);
        username[strlen(username)] = '\0';
        printf("%s\n", username);
        while (currentUser != NULL)
        {
            if (strcmp(currentUser->user.username, username) == 0)
            {
                checkUsername = 1;
                break;
            }
            currentUser = currentUser->pNext;
        }
        if (checkUsername != 1)
        {
            send_message("Cannot find account", sockfd);
            return NULL;
            break;
        }
        else
        {
            if (currentUser->user.status == BLOCKED)
            {
                send_message("Account is blocked", sockfd);
                return NULL;
                break;
            }
            else
            {
                send_message("Insert password", sockfd);
                recv(sockfd, password, MESSAGE_SIZE, 0);
                password[strlen(password)] = '\0';
                printf("password :  %s", password);
                if (strcmp(password, currentUser->user.password) != 0)
                {
                    printf("%d", checkPassword);
                    if (checkPassword == BLOCK_THRESHOLD)
                    {
                        currentUser->user.status = BLOCKED;
                        printf("check");
                        saveData(root);
                        send_message("Account is blocked", sockfd);
                        return NULL;
                        break;
                    }
                    else
                    {
                        checkPassword++;
                        send_message("Password is incorrect", sockfd);
                    }
                }
                else
                {
                    send_message("OK", sockfd);
                    currentUser->isOnline = 1;
                    return currentUser;
                    break;
                }
            }
        }
    } while (checkPassword < BLOCK_THRESHOLD + 1);
    return currentUser;
}

void logout(Node *root, int sockfd, struct sockaddr_in client_addr)
{
    char logout_message[MESSAGE_SIZE];
    Node *currentUser = root;

    printf("Logging out...\n");
    while (currentUser != NULL)
    {
        if (currentUser->isOnline && memcmp(&currentUser->user, &client_addr, sizeof(client_addr)) == 0)
        {
            currentUser->isOnline = 0;
            break;
        }
        currentUser = currentUser->pNext;
    }

    send_message("goodbye", sockfd);

    printf("Connection closed for %s\n", inet_ntoa(client_addr.sin_addr));
}

void changePass(Node *root, Node **current, int sockfd, struct sockaddr_in client_addr, char *newPass, char *name)
{
    socklen_t addr_size = sizeof(client_addr);
    char alphaString[50] = "";
    char numString[50] = "";
    int checkUsername = 0;
    Node *currentUser = root;
    while (currentUser != NULL)
    {
        if (strcmp(currentUser->user.username, name) == 0)
        {
            checkUsername = 1;
            break;
        }
        currentUser = currentUser->pNext;
    }
    if (checkUsername != 1)
    {
        send_message("Cannot find account", sockfd);
    }
    else
    {
        strcpy(currentUser->user.password, newPass);
        saveData(root);
    }
    for (int i = 0; newPass[i] != '\0'; i++)
    {
        if (isalpha(newPass[i]))
        {
            strncat(alphaString, &newPass[i], 1);
        }
        else if (isdigit(newPass[i]))
        {
            strncat(numString, &newPass[i], 1);
        }
        else
        {
            char response[MESSAGE_SIZE];
            strcpy(response, "Invalid password format.");
            send_message(response, sockfd);
            return;
        }
    }
    send_message(alphaString, sockfd);
    send_message(numString, sockfd);
}

typedef struct ThreadArgs
{
    Node *root;
    int sockfd;
    struct sockaddr_in client_addr;
} ThreadArgs;

void *handleClient(void *args)
{
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    Node *current_account = NULL;

    while (1)
    {
        char logoutmess[MESSAGE_SIZE];
        current_account = login(threadArgs->root, threadArgs->sockfd, threadArgs->client_addr);
        while (current_account != NULL)
        {
            recv(threadArgs->sockfd, logoutmess, MESSAGE_SIZE, 0);
            printf("\n%s\n", logoutmess);
            if (current_account != NULL && strstr(logoutmess, "bye") != NULL)
            {
                logout(threadArgs->root, threadArgs->sockfd, threadArgs->client_addr);
                break;
            }
            else if (current_account != NULL && strstr(logoutmess, "bye") == NULL)
            {
                changePass(threadArgs->root, &current_account, threadArgs->sockfd, threadArgs->client_addr, logoutmess, current_account->user.username);
            }
        }
    }
    close(threadArgs->sockfd);
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    Node *root = NULL;
    root = readData(root);

    int port = atoi(argv[1]);
    if (port == -1)
    {
        perror("Invalid port number");
        exit(EXIT_FAILURE);
    }

    int listen_sock, conn_sock;
    struct sockaddr_in client_addr, server_addr;
    int sin_size;

    if (argc != 2)
    {
        printf("Wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError creating socket: ");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("\nError binding socket: ");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, 5) == -1)
    {
        perror("\nError listening: ");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    int timeout = -1;

    fds[0].fd = listen_sock;
    fds[0].events = POLLIN;

    printf("Server is listening on port %d...\n", port);

    while (1)
    {
        int activity = poll(fds, nfds, timeout);

        if (activity == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN)
        {
            sin_size = sizeof(struct sockaddr_in);
            if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, (unsigned int *)&sin_size)) == -1)
            {
                perror("Error accepting connection");
                continue;
            }

            printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr));

            if (nfds < MAX_CLIENTS + 1)
            {
                fds[nfds].fd = conn_sock;
                fds[nfds].events = POLLIN;
                nfds++;
            }
            else
            {
                fprintf(stderr, "Too many clients. Connection rejected.\n");
                close(conn_sock);
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ThreadArgs *threadArgs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
                threadArgs->root = root;
                threadArgs->sockfd = fds[i].fd;
                threadArgs->client_addr = client_addr;

                pthread_t tid;
                if (pthread_create(&tid, NULL, handleClient, (void *)threadArgs) != 0)
                {
                    perror("Failed to create thread");
                    free(threadArgs);
                    close(fds[i].fd);
                }
                nfds--;
                fds[i] = fds[nfds];
                i--;
            }
        }
    }

    emptyList(root);
    close(listen_sock);

    return 0;
}
