#include <stdio.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFF_SIZE 1024
typedef struct Account
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	struct Account *next;
} Account;

typedef struct Message
{
	char digit[BUFF_SIZE];
	char str[BUFF_SIZE];
	bool isValid;
} Message;

int numAccounts;

char *converMessage(char *buff);
int getStatus(char *username);
int checkPassword(char *username_check, char *password_check);
void updateStatus(char *username_check, int new_status);
void updatePassword(char *username_check, char *new_password);
int main(int argc, char *argv[])
{

	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	char other_password[BUFF_SIZE], new_password[BUFF_SIZE];
	int bytes_sent, bytes_received_client, bytes_received_client2, message_received_client, username_password_received_client, other_password_received_client;
	struct sockaddr_in server;					  
	struct sockaddr_in client; 
	int sin_size;
	char line_break[] = "\n";

	if (argc != 2)
	{
		perror("Error Input.\n");
		exit(0);
	}

	// Step 1: Construct a UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{ 
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Bind address to socket
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1])); 
	server.sin_addr.s_addr = INADDR_ANY;	
	bzero(&(server.sin_zero), 8);			

	if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("\nError");
		exit(0);
	}

	// Step 3: Communicate with clients
	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);

		bytes_received_client = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
		if (bytes_received_client < 0)
			perror("\nError: ");
		else
		{
			printf("Client connected from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
			bytes_sent = sendto(server_sock, "done", 4, 0, (struct sockaddr *)&client, sin_size); 
			if (bytes_sent < 0)
				perror("\nError");
			//============================================================
			while (1)
			{
				username_password_received_client = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
				if (username_password_received_client < 0)
					perror("\nError");
				else
				{
					buff[username_password_received_client - 1] = '\0';
					char *newline_pos = strchr(buff, '\n');

					if (newline_pos != NULL)
					{
						*newline_pos = '\0';

						char username[BUFF_SIZE];
						strcpy(username, buff);
						char *password = newline_pos + 1;
						int count = 0;

						// Username not exist
						if (checkPassword(username, password) == 2)
						{
							bytes_sent = sendto(server_sock, "username_not_exist", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
							exit(0);
						}

						// Blocked, not ready
						if (checkPassword(username, password) == 3)
						{
							bytes_sent = sendto(server_sock, "not_ready", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
							exit(0);
						}

						// Inccorect password
						while (checkPassword(username, password) == 0 && count < 3) // TODO: Fix username not exist
						{
							bytes_sent = sendto(server_sock, "incorrect_password", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
							if (bytes_sent < 0)
								perror("\nError");
							other_password_received_client = recvfrom(server_sock, password, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
							password[other_password_received_client - 1] = '\0';
							count++;
						}

						// Inccorect password: 3 times
						if (count == 3)
						{
							bytes_sent = sendto(server_sock, "notOK", 6, 0, (struct sockaddr *)&client, sin_size);
							updateStatus(username, 0);
						}

						// OK
						int status = getStatus(username);
						if (status == 1)
						{
							bytes_sent = sendto(server_sock, "OK", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
							message_received_client = recvfrom(server_sock, new_password, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
							new_password[message_received_client - 1] = '\0';

							// Signout
							if (strcmp(new_password, "bye") == 0)
							{
								bytes_sent = sendto(server_sock, "Goodbye", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
								exit(0);
							}
							char *mess = converMessage(new_password);

							// Contain invalid character
							if (strlen(mess) == 0)
							{
								bytes_sent = sendto(server_sock, "error_digit", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);
								exit(0);
							}
							else // Change password
							{
								bytes_sent = sendto(server_sock, mess, strlen(mess), 0, (struct sockaddr *)&client, sin_size);
								updatePassword(username, new_password);
								bytes_sent = sendto(server_sock, "exit", BUFF_SIZE, 0, (struct sockaddr *)&client, sin_size);

							}
						}
						else if (status == 0 || status == 2)
						{
							bytes_sent = sendto(server_sock, "account_not_ready", 18, 0, (struct sockaddr *)&client, sin_size);
						}
					}
				}
			}
		}
	}

	close(server_sock);
	return 0;
}
void updateStatus(char *username_check, int new_status)
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
void updatePassword(char *username_check, char *new_password)
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
char *converMessage(char *buff)
{
	char digit[BUFF_SIZE];
	char str[BUFF_SIZE];
	int digit_index = -1, str_index = -1;
	int length = strlen(buff);
	for (int i = 0; i < length; i++)
	{
		if (buff[i] <= '9' && buff[i] >= '0')
		{
			digit_index++;
			digit[digit_index] = buff[i];
		}
		else if ((buff[i] <= 'z' && buff[i] >= 'a') || (buff[i] <= 'Z' && buff[i] >= 'A'))
		{
			str_index++;
			str[str_index] = buff[i];
		}
		else
		{
			return "";
		}
	}
	digit[digit_index + 1] = '\0';
	str[str_index + 1] = '\0';
	char *tmp = strcat(digit, "\n");
	tmp = strcat(tmp, str);
	return tmp;
}

int getStatus(char *username_check)
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	FILE *inp = fopen("user.txt", "r");
	do
	{
		if (fscanf(inp, "%s %s %d", username, password, &status) > 0)
		{
			if (!strcmp(username, username_check))
			{
				return status;
			}
		}
	} while (1);
}

int checkPassword(char *username_check, char *password_check)
{
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	int status;
	FILE *inp = fopen("user.txt", "r");

	while (fscanf(inp, "%s %s %d", username, password, &status) > 0)
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

	return 2;
	fclose(inp);
}