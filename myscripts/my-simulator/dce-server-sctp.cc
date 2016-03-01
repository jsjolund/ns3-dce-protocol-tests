#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define SERVER_PORT 3007
#define MAX_STREAMS 512
#define MAX_ATTEMPTS 1024
#define MAX_CLIENTS 1024

using namespace std;

int main(int argc, char **argv) {
	int master_socket, new_socket, client_socket[MAX_CLIENTS], client_reads[MAX_CLIENTS], max_clients = MAX_CLIENTS, i, bytes_read, sd;
	int max_sd;
	struct sockaddr_in address;
	struct sctp_initmsg s_initmsg;
	struct sctp_sndrcvinfo s_sndrcvinfo;
	fd_set readfds;
	char buffer[1025];
	memset(client_socket, 0, sizeof(client_socket));
	memset(client_reads, 0, sizeof(client_reads));
	memset(&address, 0, sizeof(address));
	// Create SCTP master socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
		perror("SCTP: Socket failed\n");
		exit(EXIT_FAILURE);
	}
	// Setup address
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(SERVER_PORT);

	// Bind and listen to master socket, allow 5 pending connections
	if (bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("SCTP: Bind failed\n");
		exit(EXIT_FAILURE);
	}
	// SCTP parameters
	memset(&s_initmsg, 0, sizeof(s_initmsg));
	s_initmsg.sinit_num_ostreams = MAX_STREAMS;
	s_initmsg.sinit_max_instreams = MAX_STREAMS;
	s_initmsg.sinit_max_attempts = MAX_ATTEMPTS;
	if (setsockopt(master_socket, IPPROTO_SCTP, SCTP_INITMSG, &s_initmsg, sizeof(s_initmsg)) < 0) {
		perror("SCTP Setsockopt failed\n");
		exit(EXIT_FAILURE);
	}
	if (listen(master_socket, 5) < 0) {
		perror("SCTP: Listen failed\n");
		exit(EXIT_FAILURE);
	}
	printf("SCTP: Listening on port %d\n", SERVER_PORT);
	while (1) {
		// Rebuild socket descriptor set
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		// Add all valid sockets to the set, find max socket number
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (sd > 0)
				FD_SET(sd, &readfds);
			if (sd > max_sd)
				max_sd = sd;
		}
		// Block until socket activity detected
		if ((select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0)) {
			perror("SCTP: Select failed\n");
		}
		// If master socket was selected, we have an incoming connection
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &address)) < 0) {
				perror("SCTP: Accept failed\n");
			}
			printf("SCTP: New connection: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			// Add new socket descriptor 
			for (i = 0; i < max_clients; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					break;
				}
			}
		}
		// Check other sockets for activity
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (FD_ISSET(sd, &readfds)) {
				getpeername(sd, (struct sockaddr*) &address, (socklen_t*) &address);
				// Check if it was for closing , and also read the incoming message
				bytes_read = sctp_recvmsg(sd, (void *) buffer, sizeof(buffer), (struct sockaddr *) NULL, 0, &s_sndrcvinfo, 0);
				if (bytes_read == 0) {
					printf("SCTP: Disconnected: %s:%d total %d bytes\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), client_reads[i]);
					close(sd);
					client_socket[i] = 0;
					client_reads[i] = 0;
				} else {
					printf("SCTP: Read %d bytes from %s:%d\n", bytes_read, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					client_reads[i] += bytes_read;
				}

			}
		}
	}
	return 0;
}

