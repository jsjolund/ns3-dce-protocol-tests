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

using namespace std;

void send_chars(int sock, int num_streams, int ttl, int flags, int num_chars) {
	int stat;
	int stream_i = 0;
	SenderContent content(num_chars);
	int buffer_size = 1025;
	char buffer[buffer_size];
	while (content.fill(buffer, buffer_size)) {
		stat = sctp_sendmsg(sock, buffer, (size_t) strlen(buffer), NULL, 0, 0, flags, stream_i, ttl, 0);
		stream_i = (stream_i + 1) % num_streams;
	}
}

int main(int argc, char **argv) {

	const int MAX_STREAMS = 512;
	int sock_server, connect_sock, stat, slen, i;
	struct sctp_initmsg initmsg;
	struct sockaddr_in server_addr;
	struct sctp_event_subscribe s_events;
	struct sctp_status s_status;

	// Parse input arguments 
	unsigned int bytes_to_transfer = 0;
	unsigned int ttl = 0;
	unsigned int flags = 0;
	unsigned int num_streams = 1;
	char* receiver_ip = (char *) "\0";

	while ((i = getopt(argc, argv, "a:d:t:s:u")) != -1) {
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
		default:
			break;
		}
	}

	// Create a socket
	connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	// SCTP parameters
	memset(&initmsg, 0, sizeof(initmsg));
	// TODO: These should be sent as function parameters probably
	initmsg.sinit_num_ostreams = MAX_STREAMS;         // Number of Output Stream
	initmsg.sinit_max_instreams = MAX_STREAMS;      // Number of Input Stream
	initmsg.sinit_max_attempts = MAX_STREAMS;
	stat = setsockopt(connect_sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
	if (stat < 0) {
		perror("setsockopt error\n");
		exit(-1);
	}

	// Cnnect to server IP
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(receiver_ip);
	stat = connect(connect_sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (stat < 0) {
		perror("connect error\n");
		exit(-1);
	}

	memset(&s_events, 0, sizeof(s_events));
	s_events.sctp_data_io_event = 1;

	stat = setsockopt(connect_sock, SOL_SCTP, SCTP_EVENTS, (const void *) &s_events, 9);
	if (stat < 0) {
		perror("event error\n");
		exit(-1);
	}

	slen = sizeof(s_status);
	stat = getsockopt(connect_sock, SOL_SCTP, SCTP_STATUS, (void *) &s_status, (socklen_t *) &slen);

	printf("assoc id  = %d\n", s_status.sstat_assoc_id);
	printf("state     = %d\n", s_status.sstat_state);
	printf("instrms   = %d\n", s_status.sstat_instrms);
	printf("outstrms  = %d\n", s_status.sstat_outstrms);

	send_chars(connect_sock, num_streams, ttl, flags, bytes_to_transfer);

	close(connect_sock);

	return 0;

}

