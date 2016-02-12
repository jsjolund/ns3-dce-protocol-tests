#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> 

#define SERVER_PORT 3007

int main(int argc, char *argv[]) {
	int status, sock, sock_accept;
	struct sockaddr_in addr;
	char buffer[1024];

	// Create a socket
	sock = socket(PF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	status = bind(sock, (const struct sockaddr *) &addr, sizeof(addr));
	if (status < 0) {
		perror("TCP socket bind error\n");
		exit(-1);
	}
	status = listen(sock, 1);
	if (status < 0) {
		perror("TCP listen error\n");
		exit(-1);
	}
	// Listen for incoming transmissions
	while (1) {
		printf("TCP server accepting\n");
		sock_accept = accept(sock, 0, 0);
		status = read(sock_accept, (void *) buffer, sizeof(buffer));
	}
	close(sock);
	close(sock_accept);
	return 0;
}
