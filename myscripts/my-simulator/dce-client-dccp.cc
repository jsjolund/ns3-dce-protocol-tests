#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#include "SenderContent.h"

#define SERVER_PORT 3007
#define SOL_DCCP 269

void missed_alarm(int signum) {
	// Missed timer signal, but take no action
	perror("DCCP: Missed timer.");
}

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in address;
	int current_socket, current_cycle, sock, status, result, num_sockets, op;

	// Last buffer byte holds null terminator to give 1024 data size in packet.
	int buffer_size = 1025;
	char buffer[buffer_size];

	unsigned int bytes_to_transfer = 0;
	char* receiver_ip = (char *) "\0";
	int num_cycles = 1;
	int time_between_cycles = 0;
	int send_rate_kbytes_sec = 1024;

	// Parse input arguments
	while ((op = getopt(argc, argv, "r:p:n:b:a:d:t:s:u")) != -1) {
		switch (op) {
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
		case 'r':
			send_rate_kbytes_sec = atoi(optarg);
			break;
		default:
			break;
		}
	}
	// Calculate wait time between packages 
	// Bytes sent = Buffer size + Size of Ethernet frame + Size of IPv4 Header + Size of DCCP header
	// Unit conversion: [us] * [10^6 / 2^10] = [b] / [kb/s]
	int wait_time_us = (int)((((double) buffer_size + 92) / (double) send_rate_kbytes_sec) * 976.5625);

	// Set up a timer for when to send packets
	struct itimerval timer;
	timer.it_interval.tv_sec = timer.it_value.tv_sec = 0;
	timer.it_interval.tv_usec = timer.it_value.tv_usec = wait_time_us;
	if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
		perror("DCCP: settimer failed");
		exit(1);
	}
	sigset_t alarm_sig;
	int signum;
	sigemptyset(&alarm_sig);
	sigaddset(&alarm_sig, SIGALRM);
	signal(SIGALRM, missed_alarm);

	fd_set writefds;
	FD_ZERO(&writefds);

	// Open the requested amount of sockets to distribute data on
	int sockets[num_sockets];
	for (current_socket = 0; current_socket < num_sockets; current_socket++) {
		sockets[current_socket] = socket(PF_INET, SOCK_DCCP, IPPROTO_DCCP);
		address.sin_family = AF_INET;
		address.sin_port = htons(SERVER_PORT);
		host = gethostbyname(receiver_ip);
		memcpy(&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
		if (connect(sockets[current_socket], (const struct sockaddr *) &address, sizeof(address)) < 0) {
			perror("DCCP: Connect failed\n");
			exit(EXIT_FAILURE);
		}
	}

	// Loop through connected sockets and send the specified amount of characters
	current_socket = 0;
	for (current_cycle = 0; current_cycle < num_cycles; current_cycle++) {
		SenderContent content(bytes_to_transfer);
		while (content.fill(buffer, buffer_size)) {
			FD_ZERO(&writefds);
			FD_SET(sockets[current_socket], &writefds);
			do {
				// http://www.linuxfoundation.org/collaborate/workgroups/networking/dccp
				result = select(sockets[current_socket] + 1, NULL, &writefds, NULL, NULL);
			} while (result <= 0 && (errno == EINTR || errno == EAGAIN));

			sigwait(&alarm_sig, &signum);
			write(sockets[current_socket], buffer, (size_t) strlen(buffer));
			current_socket = (current_socket + 1) % num_sockets;
		}
		if (time_between_cycles > 0)
			usleep(time_between_cycles);
	}
	for (current_socket = 0; current_socket < num_sockets; current_socket++) {
		close(sockets[current_socket]);
	}
	return 0;
}

