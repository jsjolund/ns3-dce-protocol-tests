#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "SenderContent.h"

#define SOL_DCCP 269

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in address;
	int sock, i, status, result;
	
	// Last buffer byte holds null terminator to give 1024 data size in packet.
	int buffer_size = 1025;
	char buffer[buffer_size];
	
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
	sock = socket(PF_INET, SOCK_DCCP, IPPROTO_DCCP);

	address.sin_family = AF_INET;
	address.sin_port = htons(3007);
	host = gethostbyname(receiver_ip);
	memcpy(&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	if (connect(sock, (const struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("DCCP: Connect failed\n");
		exit(EXIT_FAILURE);
	}

	ssize_t tot = 0;
	fd_set w_fds;
	FD_ZERO(&w_fds);
	FD_SET(sock, &w_fds);
	
	SenderContent content(bytes_to_transfer);
	//~ for (uint32_t i = 0; i < 1000; i++) {
	while (content.fill(buffer, buffer_size)) {
		ssize_t n = 1024;
		while (n > 0) {
			result = select(sock + 1, NULL, &w_fds, NULL, NULL);
			if (result == 0) {
				std::cout << "timeout" << std::endl;
				continue;
			}
			if (result < 0) {
				if (errno == EINTR || errno == EAGAIN) {
					std::cout << "result < 0: " << errno << std::endl;
					continue;
				}
				perror("select");
				break;
			}
			if (!FD_ISSET(sock, &w_fds)) {
				std::cout << "fd isn't set" << std::endl;
				continue;
			}
			ssize_t e = write(sock, &(buffer[1024 - n]), n);
			if (e < 0) {
				std::cout << "e < 0 : " << strerror(errno) << std::endl;
				break;
			}
			if (e < n) {
				//  sleep (1);
				std::cout << "e < n : " << e << "<" << n << std::endl;
			}
			n -= e;
			tot += e;
		}
		//    std::cout << "write: " << n << std::endl;
		usleep(100);
	}
	std::cout << "did write all buffers total:" << tot << std::endl;
	close (sock);
	
	
	//~ fd_set w_fds;
	//~ // http://www.linuxfoundation.org/collaborate/workgroups/networking/dccp
	//~ // Send the specified amount of characters
	//~ SenderContent content(bytes_to_transfer);
	//~ while (content.fill(buffer, buffer_size)) {
		//~ while (1) {
			//~ FD_ZERO(&w_fds);
			//~ FD_SET(sock, &w_fds);
			//~ do {
				//~ result = select(sock + 1, NULL, &w_fds, NULL, NULL);
			//~ } while (result <= 0 && (errno == EINTR || errno == EAGAIN));
			//~ if (!FD_ISSET(sock, &w_fds)) {
				//~ std::cout << "fd isn't set" << std::endl;
				//~ continue;
			//~ }
			//~ ssize_t e = write(sock, buffer, (size_t) strlen(buffer));
			//~ if (e < 0) {
				//~ std::cout << "e < 0 : " << strerror(errno) << std::endl;
				//~ continue;
			//~ }
			//~ break;
		//~ }
		//~ usleep(100);
	//~ }
	//~ close(sock);

	return 0;
}

