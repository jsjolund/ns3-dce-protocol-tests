// 
// libstcp1-dev is needed
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>        /* for memset */
#include <unistd.h>        /* for memset */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

int main(int argc, char **argv) {
	const int MAX_STREAMS = 512;
	int connect_sock, stat, port, slen, i, flags;
	struct sctp_initmsg initmsg;
	struct sockaddr_in server_addr;
	struct sctp_event_subscribe s_events;
	struct sctp_status s_status;
	struct sctp_sndrcvinfo s_sndrcvinfo;
	char buffer[1024];

	port = 3007;

	connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	memset(&initmsg, 0, sizeof(initmsg));

	// TODO: These should be sent as function parameters probably
	initmsg.sinit_num_ostreams = MAX_STREAMS;          // Number of Output Stream
	initmsg.sinit_max_instreams = MAX_STREAMS;      // Number of Input Stream
	initmsg.sinit_max_attempts = MAX_STREAMS;
	stat = setsockopt(connect_sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
	if (stat < 0) {
		perror("setsockopt error\n");
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	stat = connect(connect_sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (stat < 0) {
		perror("connect error\n");
		exit(-1);
	}

	memset(&s_events, 0, sizeof(s_events));
	s_events.sctp_data_io_event = 1;

	// The size of events is 9, you can get the struct sctp_event_subscribe from the sctp.h. 
	// But as described in the section 9.14 of Unix Network Programming Volume 1, 
	// there are only 8 events in the struct sctp_event_subscribe, not 9. 
	// So if you set the value to 9 (sizeof(s_events)), it will be error.
	stat = setsockopt(connect_sock, SOL_SCTP, SCTP_EVENTS, (const void *) &s_events, 9);
	if (stat < 0) {
		perror("event error\n");
		exit (-1);
	}

	slen = sizeof(s_status);
	stat = getsockopt(connect_sock, SOL_SCTP, SCTP_STATUS, (void *) &s_status, (socklen_t *) &slen);

	printf("assoc id  = %d\n", s_status.sstat_assoc_id);
	printf("state     = %d\n", s_status.sstat_state);
	printf("instrms   = %d\n", s_status.sstat_instrms);
	printf("outstrms  = %d\n", s_status.sstat_outstrms);

	while (1) {
		stat = sctp_recvmsg(connect_sock, (void *) buffer, sizeof(buffer), 
				(struct sockaddr *) NULL, 0, &s_sndrcvinfo, &flags);
		printf("client stat = %i\n", stat);
	}
	/* Close our socket and exit */
	close(connect_sock);
	return 0;
}

