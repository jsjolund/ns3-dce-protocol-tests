#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "SenderContent.h"

#define SERVER_PORT 3007

using namespace std;

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in address;
	int i, status, num_sockets;

	// Last buffer byte holds null terminator to give 1024 data size in packet.
	int buffer_size = 1025;
	char buffer[buffer_size];

	unsigned int bytes_to_transfer = 0;
	char* receiver_ip = (char *) "\0";
	int num_cycles = 1;
	int time_between_cycles = 0;

	// Parse input arguments 
	while ((i = getopt(argc, argv, "p:n:b:a:d:t:s:u")) != -1) {
		switch (i) {
		case 'a':
			receiver_ip = optarg;
			break;
		case 'd':
			bytes_to_transfer = atoi(optarg);
			break;
		case 's':
			num_sockets = atoi(optarg);
			break;
		case 'n':
			num_cycles = atoi(optarg);
			break;
		case 'b':
			time_between_cycles = atoi(optarg);
			break;
		default:
			break;
		}
	}
	int sockets[num_sockets];

	// Open the requested amount of sockets to distribute data on
	for (i = 0; i < num_sockets; i++) {
		sockets[i] = socket(PF_INET, SOCK_STREAM, 0);
		address.sin_family = AF_INET;
		address.sin_port = htons(SERVER_PORT);
		host = gethostbyname(receiver_ip);
		memcpy(&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
		if (connect(sockets[i], (const struct sockaddr *) &address, sizeof(address)) < 0) {
			perror("TCP: Connect failed\n");
			exit(EXIT_FAILURE);
		}
	}

	// Send the specified amount of characters
	i = 0;
	int j;
	for (j = 0; j < num_cycles; j++) {
		SenderContent content(bytes_to_transfer);
		while (content.fill(buffer, buffer_size)) {
			send(sockets[i], buffer, (size_t) strlen(buffer), 0);
			i = (i + 1) % num_sockets;
		}
		if (time_between_cycles > 0)
			usleep(time_between_cycles);
	}
	for (i = 0; i < num_sockets; i++) {
		close(sockets[i]);
	}
	return 0;
}
