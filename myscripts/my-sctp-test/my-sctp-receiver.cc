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

using namespace std;

int main(int argc, char **argv) {
	const int MAX_STREAMS = 512;
	int sock, sock_accept, status, i;
	struct sockaddr_in server_addr;
	struct sctp_initmsg s_initmsg;
	struct sctp_sndrcvinfo s_sndrcvinfo;
	char buffer[1024];

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	status = bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));

	// SCTP parameters
	memset(&s_initmsg, 0, sizeof(s_initmsg));
	s_initmsg.sinit_num_ostreams = MAX_STREAMS;
	s_initmsg.sinit_max_instreams = MAX_STREAMS;
	s_initmsg.sinit_max_attempts = MAX_STREAMS;
	status = setsockopt(sock, IPPROTO_SCTP, SCTP_INITMSG, &s_initmsg, sizeof(s_initmsg));
	if (status < 0) {
		perror("SCTP setsockopt error\n");
		exit(-1);
	}
	status = listen(sock, 5); // http://linux.die.net/man/2/listen
	if (status < 0) {
		perror("SCTP socket listen error\n");
		exit(-1);
	}
	// Listen for incoming transmissions
	while (1) {
		printf("SCTP server accepting\n");
		sock_accept = accept(sock, (struct sockaddr *) NULL, (socklen_t *) NULL);
		status = sctp_recvmsg(sock_accept, (void *) buffer, sizeof(buffer), (struct sockaddr *) NULL, 0, &s_sndrcvinfo,
				0);
	}
	close(sock);
	close(sock_accept);
	return 0;
}

