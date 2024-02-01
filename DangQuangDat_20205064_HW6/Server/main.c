#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFF_SIZE 1024

typedef struct Message
{
	char digital[BUFF_SIZE];
	char str[BUFF_SIZE];
	bool checkValid;
} Message;

typedef struct Account
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	struct Account *next;
} Account;

void changeStt(char *username_check, int new_status);
void changePass(char *username_check, char *new_password);
char *convertMess(char *buff);

int getStt(char *username);
int checkValidPassWord(char *username_check, char *password_check);

char line_break[] = "\n";

pid_t pid;
pid_t wait(int *statloc);
pid_t waitpid(pid_t pid, int *statloc, int options);

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	pid = waitpid(-1, &stat, WNOHANG);
	printf("child %d terminated\n", pid);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int server_sock;
	char buff[BUFF_SIZE];
	char otherPassword[BUFF_SIZE], new_password[BUFF_SIZE];
	int byte_send, byte_received_client, byte_received_client2, message_received, username_password_received, otherPassword_received;
	char goodbye[] = "Goodbye";
	char not_ready[] = "not_ready";
	char username_not_exist[] = "username_not_exist";
	char error_digital[] = "error_digital";
	int sin_sz;

	if (argc != 2)
	{
		perror("Error Input.\n");
		exit(0);
	}
	server_sock = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));
	bzero(&(server_addr.sin_zero), 8);

	bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	listen(server_sock, 5);

	signal(SIGCHLD, sig_chld);

	while (1)
	{
		sin_sz = sizeof(struct sockaddr_in);

		int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_sz); // Chấp nhận kết nối từ client_addr

		if ((pid = fork()) == 0)
		{
			close(server_sock);

			if (client_sock < 0)
			{
				perror("\nError accepting connection");
				exit(0);
			}
			printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

			int bytes_sent = send(client_sock, "done", 4, 0);
			if (bytes_sent < 0)
				perror("\nError");

			while (1)
			{
				username_password_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
				if (username_password_received < 0)
				{
					perror("\nError: ");
					break;
				}
				else
				{
					buff[username_password_received - 1] = '\0';
					char *newline_pos = strchr(buff, '\n');

					if (newline_pos != NULL)
					{
						*newline_pos = '\0';

						char username[BUFF_SIZE];
						strcpy(username, buff);
						char *password = newline_pos + 1;
						int count = 1;

						if (checkValidPassWord(username, password) == 2)
						{
							byte_send = send(client_sock, username_not_exist, BUFF_SIZE, 0);
							break;
						}

						if (checkValidPassWord(username, password) == 3)
						{
							byte_send = send(client_sock, not_ready, BUFF_SIZE, 0);
							break;
						}

						while (checkValidPassWord(username, password) == 0 && count <= 2)
						{
							byte_send = send(client_sock, "incorrect_password\n", BUFF_SIZE, 0);

							otherPassword_received = recv(client_sock, password, BUFF_SIZE - 1, 0);
							password[otherPassword_received - 1] = '\0';
							count++;
						}

						if (count == 3)
						{
							changeStt(username, 0);
							byte_send = send(client_sock, "notOK", 6, 0);
						}

						int status = getStt(username);

						if (status == 0 || status == 2)
						{
							byte_send = send(client_sock, "account_not_ready", 18, 0);
						}
						else if (status == 1)
						{
							byte_send = send(client_sock, "OK", BUFF_SIZE, 0);
							message_received = recv(client_sock, new_password, BUFF_SIZE - 1, 0);
							new_password[message_received - 1] = '\0';

							// Signout
							if (strcmp(new_password, "bye") == 0)
							{
								byte_send = send(client_sock, goodbye, BUFF_SIZE, 0);
								break;
							}
							char *message = convertMess(new_password);

							if (strlen(message) == 0)
							{
								byte_send = send(client_sock, error_digital, BUFF_SIZE, 0);
								break;
							}
							else
							{
								byte_send = send(client_sock, message, strlen(message), 0);
								changePass(username, new_password);
								byte_send = send(client_sock, "exit", BUFF_SIZE, 0);
							}
						}
					}
				}
			}
			close(client_sock);
			exit(0);
		}

		close(client_sock);
	}
	return 0;
}

char *convertMess(char *buffer)
{
	char digital[BUFF_SIZE];
	char str[BUFF_SIZE];
	int digital_index = -1, str_index = -1;
	int length = strlen(buffer);
	for (int i = 0; i < length; i++)
	{

		if ((buffer[i] <= 'z' && buffer[i] >= 'a') || (buffer[i] <= 'Z' && buffer[i] >= 'A'))
		{
			str_index++;
			str[str_index] = buffer[i];
		}
		else if (buffer[i] <= '9' && buffer[i] >= '0')
		{
			digital_index++;
			digital[digital_index] = buffer[i];
		}
		else
		{
			return "";
		}
	}
	digital[digital_index + 1] = '\0';
	str[str_index + 1] = '\0';
	char *tmp = strcat(digital, "\n");
	tmp = strcat(tmp, str);
	return tmp;
}

int checkValidPassWord(char *username_check, char *password_check)
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	FILE *file = fopen("user.txt", "r");

	while (fscanf(file, "%s %s %d", username, password, &status) > 0)
	{
		if (strcmp(username, username_check) == 0)
		{
			if (strcmp(password, password_check) == 0)
			{
				if (status != 1)
				{
					return 3;
				}
				return 1;
			}
			else
				return 0;
		}
	}

	fclose(file);
	return 2;
}
void changePass(char *username_check, char *new_password)
{
	const char *filename = "user.txt";
	FILE *file = fopen(filename, "r");
	FILE *tempFile = fopen("temp.txt", "w");
	char line[100];
	while (fgets(line, sizeof(line), file))
	{
		char username[BUFF_SIZE];
		char password[BUFF_SIZE];
		int status;
		if (sscanf(line, "%s %s %d", username, password, &status) == 3)
		{
			if (strcmp(username, username_check) == 0)
			{
				strcpy(password, new_password);
			}

			// rewrite tmp file
			fprintf(tempFile, "%s %s %d\n", username, password, status);
		}
	}

	fclose(file);
	fclose(tempFile);
	remove(filename);
	rename("temp.txt", filename);
	return;
}

int getStt(char *username_check)
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	FILE *file = fopen("user.txt", "r");
	do
	{
		if (fscanf(file, "%s %s %d", username, password, &status) > 0)
		{
			if (!strcmp(username, username_check))
			{
				return status;
			}
		}
	} while (1);
}

void changeStt(char *username_check, int new_status)
{
	const char *filename = "user.txt";
	FILE *file = fopen(filename, "r");
	FILE *tempFile = fopen("temp.txt", "w");
	char line[100];
	while (fgets(line, sizeof(line), file))
	{
		char username[BUFF_SIZE];
		char password[BUFF_SIZE];
		int status;

		if (sscanf(line, "%s %s %d", username, password, &status) == 3)
		{
			if (strcmp(username, username_check) == 0)
			{
				status = new_status;
			}

			// rewrite tmp file
			fprintf(tempFile, "%s %s %d\n", username, password, status);
		}
	}

	fclose(file);
	fclose(tempFile);
	remove(filename);
	rename("temp.txt", filename);
	return;
}
