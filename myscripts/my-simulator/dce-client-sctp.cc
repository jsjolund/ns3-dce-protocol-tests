#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include "SenderContent.h"

#define SERVER_PORT 3007
#define MAX_STREAMS 512
#define MAX_ATTEMPTS 1024

using namespace std;

int main(int argc, char **argv) {
	int sock_server, connect_sock, stat, i, slen;
	struct sctp_initmsg initmsg;
	struct sockaddr_in server_addr;
	struct sctp_event_subscribe s_events;
	struct sctp_rtoinfo s_rto;
	struct sctp_status s_status;

	// Last buffer byte holds null terminator to give 1024 data size in packet.
	int buffer_size = 1025;
	char buffer[buffer_size];

	// Parse input arguments 
	unsigned int bytes_to_transfer = 0;
	unsigned int ttl = 0;
	unsigned int flags = 0;
	unsigned int num_streams = 1;
	unsigned int num_cycles = 1;
	unsigned int time_between_cycles = 0;
	char* receiver_ip = (char *) "\0";

	while ((i = getopt(argc, argv, "r:p:n:b:a:d:t:s:u")) != -1) {
		switch (i) {
		case 'a':
			receiver_ip = optarg;
			break;
		case 'd':
			bytes_to_transfer = atoi(optarg);
			break;
		case 't':
			ttl = atoi(optarg);
			break;
		case 's':
			num_streams = atoi(optarg);
			break;
		case 'u':
			flags |= SCTP_UNORDERED;
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

	// Create a socket
	connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	// SCTP socket parameters. TODO: These should be sent as function parameters probably
	memset(&initmsg, 0, sizeof(initmsg));
	initmsg.sinit_num_ostreams = MAX_STREAMS;
	initmsg.sinit_max_instreams = MAX_STREAMS;
	initmsg.sinit_max_attempts = MAX_ATTEMPTS;
	if (setsockopt(connect_sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0) {
		perror("SCTP: Setsockopt failed\n");
		exit(EXIT_FAILURE);
	}
	// Connect to server IP
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(receiver_ip);
	if (connect(connect_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("SCTP: Connect failed\n");
		exit(EXIT_FAILURE);
	}
	// SCTP socket parameters
	memset(&s_events, 0, sizeof(s_events));
	s_events.sctp_data_io_event = 1;
	if (setsockopt(connect_sock, SOL_SCTP, SCTP_EVENTS, (const void *) &s_events, 9) < 0) {
		perror("SCTP: SCTP_EVENTS error\n");
		exit(EXIT_FAILURE);
	}
	
	//~ // SCTP retransmission timeout, not used
	//~ memset(&s_rto, 0, sizeof(s_rto));
	//~ s_rto.srto_initial = 3000;
	//~ s_rto.srto_min = 1000;
	//~ s_rto.srto_max = 60000;
	//~ if (setsockopt(connect_sock, SOL_SCTP, SCTP_RTOINFO, (const void *) &s_rto, sizeof(s_rto)) < 0) {
	//~ perror("SCTP: SCTP_RTOINFO error\n");
	//~ exit(EXIT_FAILURE);
	//~ }

	// Print SCTP connection status
	int s_status_len = sizeof(s_status);
	getsockopt(connect_sock, SOL_SCTP, SCTP_STATUS, (void *) &s_status, (socklen_t *) &s_status_len);
	printf("assoc id  = %d\n", s_status.sstat_assoc_id);
	printf("state     = %d\n", s_status.sstat_state);
	printf("instrms   = %d\n", s_status.sstat_instrms);
	printf("outstrms  = %d\n", s_status.sstat_outstrms);

	// Send the specified amount of characters, distribute them by 1024 bytes on each stream
	int stream_i = 0, j;
	for (j = 0; j < num_cycles; j++) {
		SenderContent content(bytes_to_transfer);
		while (content.fill(buffer, buffer_size)) {
			sctp_sendmsg(connect_sock, buffer, (size_t) strlen(buffer), NULL, 0, 0, flags, stream_i, ttl, 0);
			stream_i = (stream_i + 1) % num_streams;
		}
		if (time_between_cycles > 0)
			usleep(time_between_cycles);
	}
	close(connect_sock);
	return 0;

}

