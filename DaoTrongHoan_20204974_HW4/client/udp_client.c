#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
	int client_sock;
	char buff[BUFF_SIZE], connected[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received, sin_size;

	if (argc != 3)
	{
		perror("Error Input.\n");
		exit(0);
	}

	// Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("\nError: ");
		exit(0);
	}

	// Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sin_size = sizeof(struct sockaddr);

	// Connect to server
	char message[] = "connect";
	bytes_sent = sendto(client_sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sin_size);
	if (bytes_sent < 0)
	{
		perror("Error: ");
		close(client_sock);
		return 0;
	}

	bytes_received = recvfrom(client_sock, connected, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
	if (bytes_received < 0)
	{
		perror("Error: ");
		close(client_sock);
		return 0;
	}
	connected[bytes_received] = '\0';
	if (strcmp(connected, "done") == 0)
	{
		printf("\nInput username: ");
		memset(buff, '\0', (strlen(buff) + 1));
		fgets(buff, BUFF_SIZE, stdin);

		printf("Input password: ");
		char password[BUFF_SIZE];
		memset(password, '\0', (strlen(password) + 1));
		fgets(password, BUFF_SIZE, stdin);
		strcat(buff, password);

		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
		if (bytes_sent < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}
	}

	// Step 4: Communicate with server
	while (strcmp(connected, "done") == 0)
	{
		// Received messsage form server
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
		if (bytes_received < 0)
		{
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received] = '\0';
		if (strcmp(buff, "exit") == 0)
		{
			printf("Exit succesfull");
			return 0;
		}

		// Incorrect Password
		if (strcmp(buff, "incorrect_password\n") == 0)
		{
			printf("Incorrect password! Please enter other password: ");
			char password[BUFF_SIZE];
			memset(password, '\0', (strlen(password) + 1));
			fgets(password, BUFF_SIZE, stdin);

			// Gửi xâu `username\npassword` đến server
			bytes_sent = sendto(client_sock, password, strlen(password), 0, (struct sockaddr *)&server_addr, sin_size);
			if (bytes_sent < 0)
			{
				perror("Error: ");
				close(client_sock);
				return 0;
			}
		}
		// Login thanh cong
		else if (strcmp(buff, "OK") == 0)
		{
			printf("Enter a string (enter \"bye\" to exit): ");
			char buff[BUFF_SIZE];
			memset(buff, '\0', (strlen(buff) + 1));
			fgets(buff, BUFF_SIZE, stdin);
			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
		}
		else if (strcmp(buff, "Goodbye") == 0)
		{
			printf("Good bye!\n");
			exit(0);
		}
		else if (strcmp(buff, "error_digit") == 0)
		{
			printf("Error digit!\n");
			exit(0);
		}
		else if (strcmp(buff, "username_not_exist") == 0)
		{
			printf("username_not_exist!\n");
			exit(0);
		}
		else if (strcmp(buff, "not_ready") == 0)
		{
			printf("not_ready!\n");
			exit(0);
		}
		else if (strcmp(buff, "exit") == 0)
		{
			printf("Exit succesfull\n");
			return 0;
		}
		buff[bytes_received] = '\0';

		printf("Reply from server: %s\n", buff);
	}

	close(client_sock);
	return 0;
}