#include "../include/sockets.h"

int socket_create_listener(char* ip, char* port)
{
	if(ip == NULL)
		return -1;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(NULL, port, &hints, &server_info);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int activated = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activated, sizeof(activated));

	if(server_socket == -1 || bind(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);

	if(listen(server_socket, BACKLOG) == -1)
		return -1;
	return server_socket;
}


int socket_connect_to_server(char* ip, char* port)
{
	if(ip == NULL)
		return -1;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, port, &hints, &server_info);
	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int result = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return server_socket;
}
