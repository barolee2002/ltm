#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
	int client_sock;
	char buffer_send[BUFF_SIZE], connected[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int byte_sent, bytes_received, sin_size, msg_len;
	char goodbye[] = "Goodbye";
	char not_ready[] = "not_ready";
	char username_not_exist[] = "username_not_exist";
	char error_digital[] = "error_digital";

	if (argc != 3)
	{
		perror("\nError Input.\n");
		exit(0);
	}

	// Construct a TCP socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	sin_size = sizeof(struct sockaddr);

	// Connect to server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(client_sock);
		exit(0);
	}

	bytes_received = recv(client_sock, connected, BUFF_SIZE - 1, 0);
	if (bytes_received < 0)
	{
		close(client_sock);
		return 0;
	}
	connected[bytes_received] = '\0';

	if (strcmp(connected, "done") == 0)
	{
		printf("\nInsert username: ");
		memset(buffer_send, '\0', (strlen(buffer_send) + 1));
		fgets(buffer_send, BUFF_SIZE, stdin);
		msg_len = strlen(buffer_send);
		if (msg_len == 0)
		{
			return 0;
		}
		byte_sent = send(client_sock, buffer_send, msg_len, 0);
		if (byte_sent < 0)
		{
			perror("\nError: ");
			close(client_sock);
			return 0;
		}

		printf("\nInsert password: ");
		char password[BUFF_SIZE];
		memset(password, '\0', (strlen(password) + 1));
		fgets(password, BUFF_SIZE, stdin);
		strcat(buffer_send, password);

		msg_len = strlen(buffer_send);
		if (msg_len == 0)
		{
			return 0;
		}
		byte_sent = send(client_sock, buffer_send, msg_len, 0);
		if (byte_sent < 0)
		{
			perror("\nError: ");
			close(client_sock);
			return 0;
		}
	}

	// Step 4: Communicate with server
	while (1)
	{
		// Received messsage form server
		bytes_received = recv(client_sock, buffer_send, BUFF_SIZE - 1, 0);
		if (bytes_received < 0)
		{
			break;
		}
		buffer_send[bytes_received] = '\0';
		if (strcmp(buffer_send, "exit") == 0)
		{
			printf("Exit succesful\n");
			return 0;
		}

		// Incorrect Password
		if (strcmp(buffer_send, "incorrect_password\n") == 0)
		{
			printf("Incorrect password! Please enter other password: ");
			char password[BUFF_SIZE];
			memset(password, '\0', (strlen(password) + 1));
			fgets(password, BUFF_SIZE, stdin);

			// Gửi xâu `username\npassword` đến server
			byte_sent = send(client_sock, password, strlen(password), 0);
			if (byte_sent < 0)
			{
				perror("\nError: ");
				break;
			}
		}

		// Login thanh cong
		else if (strcmp(buffer_send, "OK") == 0)
		{
			printf("Enter a string (enter \"bye\" to exit): ");
			char buffer_send[BUFF_SIZE];
			memset(buffer_send, '\0', (strlen(buffer_send) + 1));
			fgets(buffer_send, BUFF_SIZE, stdin);
			byte_sent = send(client_sock, buffer_send, strlen(buffer_send), 0);
		}
		else if (strcmp(buffer_send, not_ready) == 0)
		{
			printf("Account not ready!\n");
			break;
		}
		else if (strcmp(buffer_send, username_not_exist) == 0)
		{
			printf("Username not exist!\n");
			break;
		}
		else if (strcmp(buffer_send, error_digital) == 0)
		{
			printf("Error digit!\n");
			break;
		}
		else if (strcmp(buffer_send, goodbye) == 0)
		{
			printf("Good bye\n");
			break;
		}
		else if (strcmp(buffer_send, "exit") == 0)
		{
			printf("Exit succesfull\n");
			break;
		}
		buffer_send[bytes_received] = '\0';

		printf("Reply from server: %s\n", buffer_send);
	}

	close(client_sock);
	return 0;
}
