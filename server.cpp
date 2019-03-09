#include <arpa/inet.h>
#include <cstdio>
#include <string>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	uint16_t server_port = 1234;

	struct sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(server_port);

	// Creating socket file descriptor
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd <= 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	int opt = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	int queue_limit = 3;
	if (listen(sock_fd, queue_limit) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		int addr_len = sizeof(address);
		int new_socket = accept(sock_fd, (struct sockaddr *) &address, (socklen_t *) &addr_len);
		if (new_socket < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		char buffer[1024] = {0};
		read(new_socket, buffer, 1024);
		printf("%s\n", buffer);
		std::string message{"Hello from server"};
		send(new_socket, message.c_str(), message.length(), 0);
		printf("Hello message sent\n");
	}
}
