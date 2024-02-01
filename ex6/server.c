#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>

#define MESSAGE_SIZE 1024
#define BLOCK_THRESHOLD 3

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

    close(sockfd);
    printf("Connection closed for %s\n", inet_ntoa(client_addr.sin_addr));
}
void change_password(Node *root, Node **current, int sockfd, struct sockaddr_in client_addr)
{
    do
    {
        char *newPassword;
        socklen_t addr_size = sizeof(client_addr);
        recv(sockfd, newPassword, MESSAGE_SIZE, 0);
        newPassword[strlen(newPassword)] = '\0';
        printf("%s\n", newPassword);
        char alphaString[50] = "";
        char numString[50] = "";

        // if (strcmp(newPassword, "bye") == 0)
        // {
        //     char *response = "Goodbye";
        //     send_message(response, sockfd);
        //     break;
        // }
        for (int i = 0; newPassword[i] != '\0'; i++)
        {
            if (isalpha(newPassword[i]))
            {
                strncat(alphaString, &newPassword[i], 1);
            }
            else if (isdigit(newPassword[i]))
            {
                strncat(numString, &newPassword[i], 1);
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
        strcpy((*current)->user.password, newPassword);
        saveData(root);

        char response[MESSAGE_SIZE];
        strcpy(response, "Password changed successfully.");
        send_message(response, sockfd);
    } while (1);
}
void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("Child %d terminated\n", pid);
}

int main(int argc, char *argv[])
{
    Node *root = NULL;
    root = readData(root);
    Node *current_accout = NULL;
    int port = atoi(argv[1]);
    if (port == -1)
        perror("Invalid port number");

    int listen_sock, conn_sock;
    struct sockaddr_in client_addr, server_addr;
    int received_mess, sin_size;
    if (argc != 2)
    {
        printf("Wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        return 0;
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("\nError: ");
        return 0;
    }
    if (listen(listen_sock, 5) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    signal(SIGCHLD, sig_chld);

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, (unsigned int *)&sin_size)) == -1)
        {
            perror("\nError: ");
            break;
        }
        printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr));

        pid_t pid = fork();
        if (pid == -1)
        {
            close(conn_sock);
            printf("ERROR in new process creation\n");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            close(conn_sock);
            continue;
        }
        else if (pid == 0)
        {
            close(listen_sock);
            while (1)
            {
                char logoutmess[MESSAGE_SIZE];
                current_accout = login(root, conn_sock, client_addr);
                recv(conn_sock, logoutmess, MESSAGE_SIZE, 0);
                printf("%s\n", current_accout->user.username);
                if (current_accout != NULL && strstr(logoutmess, "bye") != " ")
                {
                    logout(root, conn_sock, client_addr);
                    emptyList(root);
                    close(conn_sock);
                    break;
                } else {
                    change_password(root,&current_accout,conn_sock,client_addr);
                }
            }
            exit(EXIT_SUCCESS);
        }
    }

    emptyList(root);
    close(conn_sock);
    return 0;
}
