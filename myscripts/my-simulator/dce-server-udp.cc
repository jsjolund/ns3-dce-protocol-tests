#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define SERVER_PORT 3007
#define MAX_CLIENTS 1024

int main(int argc, char *argv[]) {
	int master_socket, client_reads[MAX_CLIENTS], i, bytes_read, sd;
	int max_sd;
	struct sockaddr_in address;
	fd_set readfds;
	char buffer[1025];
	//~ memset(client_socket, 0, sizeof(client_socket));
	memset(client_reads, 0, sizeof(client_reads));
	memset(&address, 0, sizeof(address));
	// Create UDP master socket
	if ((master_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("UDP: Socket failed\n");
		exit(EXIT_FAILURE);
	}
	// Setup address
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( SERVER_PORT);
	// Bind and listen to master socket, allow 5 pending connections
	if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("UDP: Bind failed\n");
		exit(EXIT_FAILURE);
	}
	printf("UDP: Listening on port %d\n", SERVER_PORT);
	while (1) {
		if ((bytes_read = recvfrom(master_socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &address, (socklen_t *) &address)) < 0) {
			perror("UDP: Recvfrom failed\n");
			exit(EXIT_FAILURE);
		}
		printf("UDP: Read %d bytes from %s:%d\n", bytes_read, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	}
	return 0;
}

