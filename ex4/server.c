#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
    file = fopen("nguoidung.txt", "r");
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
    file = fopen("nguoidung.txt", "wt");
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

int checkCharactor(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ')
        {
            return 0;
        }
    }
    return 1;
}

void send_message(char *message, int sockfd, struct sockaddr_in client_addr)
{
    sendto(sockfd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&client_addr,
           sizeof(client_addr));
}

void received_message(int sockfd, char *message, struct sockaddr_in *client_addr)
{
    socklen_t addr_size = sizeof(*client_addr);
    recvfrom(sockfd, message, MESSAGE_SIZE, MSG_WAITALL, (struct sockaddr *)client_addr, &addr_size);
    message[strlen(message)] = '\0';
}

Node *login(Node *root, int sockfd, struct sockaddr_in client_addr)
{
    char username[50], password[50];
    int checkUsername = 0, checkPassword = 0;
    Node *currentUser = root;
    printf("__________Login__________\n");

    do
    {

        printf("Username: ");
        received_message(sockfd, username, &client_addr);
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
            send_message("Cannot find account", sockfd, client_addr);
            return NULL;
            break;
        }
        else
        {
            if (currentUser->user.status == BLOCKED)
            {
                send_message("Account is blocked", sockfd, client_addr);
                return NULL;
                break;
            }
            else
            {
                send_message("Insert password", sockfd, client_addr);
                received_message(sockfd, password, &client_addr);
                if (strcmp(password, currentUser->user.password) != 0)
                {
                    printf("%d", checkPassword);
                    if (checkPassword == BLOCK_THRESHOLD)
                    {
                        currentUser->user.status = BLOCKED;
                        printf("check");
                        saveData(root);
                        send_message("Account is blocked", sockfd, client_addr);
                        return NULL;
                        break;
                    }
                    else
                    {

                        checkPassword++;
                        send_message("Password is incorrect", sockfd, client_addr);
                    }
                }
                else
                {
                    send_message("OK", sockfd, client_addr);
                    currentUser->isOnline = 1;
                    break;
                }
            }
        }
    } while (checkPassword < BLOCK_THRESHOLD + 1);
    return currentUser;
};
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

        if (strcmp(newPassword, "bye") == 0)
        {
            char *response = "Goodbye";
            sendto(sockfd, response, strlen(response), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
            break;
        }
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
                sendto(sockfd, response, strlen(response), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
                return;
            }
        }
        send_message(alphaString, sockfd, client_addr);
        send_message(numString, sockfd, client_addr);
        strcpy((*current)->user.password, newPassword);
        saveData(root);

        char response[MESSAGE_SIZE];
        strcpy(response, "Password changed successfully.");
        sendto(sockfd, response, strlen(response), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
    } while (1);
}
int main(int argc, char *argv[])
{
    Node *root = NULL;
    root = readData(root);
    Node *current_accout = NULL;
    int sockfd;
    char server_buffer[MESSAGE_SIZE];
    struct sockaddr_in client_addr, server_addr;
    int received_mess, sin_size;
    if (argc != 2)
    {
        printf("Wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        current_accout = login(root, sockfd, client_addr);
        if (current_accout != NULL)
        {
            change_password(root, &current_accout, sockfd, client_addr);
        }
    }

    emptyList(root);
    close(sockfd);
    return 0;
}
