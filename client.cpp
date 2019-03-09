#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int sock_fd = 0;
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr{};
	memset(&serv_addr, 0, sizeof(serv_addr));

	std::string server_ip{"127.0.0.1"};
	uint16_t server_port = 1234;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address / address not supported\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		exit(EXIT_FAILURE);
	}

	std::string message{"Hello from client"};
	send(sock_fd, message.c_str(), message.length(), 0);
	printf("Hello message sent\n");
	char buffer[1024] = {0};
	read(sock_fd, buffer, 1024);
	printf("%s\n", buffer);
}
