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

/**
 * Send the specified amount of characters to this socket.
 * @param sock The socket to send to.
 * @param num_chars Number of characters (bytes) to send.
 */
void send_chars(int sock, int num_chars) {
	int stat;
	SenderContent content(num_chars);
	int buffer_size = 1025;
	char buffer[buffer_size];
	while (content.fill(buffer, buffer_size)) {
		stat = send(sock, buffer, (size_t) strlen(buffer), 0);
	}
}

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in addr;
	int connect_sock, i, status;
	unsigned int bytes_to_transfer = 0;
	char* receiver_ip = (char *) "\0";

	// Parse input arguments 
	while ((i = getopt(argc, argv, "a:d:t:s:u")) != -1) {
		switch (i) {
		case 'a':
			receiver_ip = optarg;
			break;
		case 'd':
			bytes_to_transfer = atoi(optarg);
			break;
		default:
			break;
		}
	}
	// Create a socket and connect to server IP
	connect_sock = socket(PF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	host = gethostbyname(receiver_ip);
	memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	status = connect(connect_sock, (const struct sockaddr *) &addr, sizeof(addr));
	if (status < 0) {
		perror("TCP connect error\n");
		exit(-1);
	}
	// Send the specified amount of characters
	send_chars(connect_sock, bytes_to_transfer);
	close(connect_sock);
	return 0;
}
