#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
	const int MAX_STREAMS = 512;
	int sock_listen, sock_server, stat, i;
	struct sockaddr_in server_addr;
	struct sctp_initmsg s_initmsg;
	int echo_port = 3007;
	struct sctp_sndrcvinfo s_sndrcvinfo;
	char buffer[1024];
	
	sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(echo_port);

	stat = bind(sock_listen, (struct sockaddr *) &server_addr, sizeof(server_addr));

	// SCTP parameter
	memset(&s_initmsg, 0, sizeof(s_initmsg));
	s_initmsg.sinit_num_ostreams = MAX_STREAMS;
	s_initmsg.sinit_max_instreams = MAX_STREAMS;
	s_initmsg.sinit_max_attempts = MAX_STREAMS;

	stat = setsockopt(sock_listen, IPPROTO_SCTP, SCTP_INITMSG, &s_initmsg, sizeof(s_initmsg));
	if (stat < 0) {
		perror("Socket Option error");
		exit(-1);
	}

	listen(sock_listen, 5); // http://linux.die.net/man/2/listen

	while (1) {
		printf("SCTP server accepting\n");
		sock_server = accept(sock_listen, (struct sockaddr *) NULL, (socklen_t *) NULL);
		stat = sctp_recvmsg(sock_server, (void *) buffer, sizeof(buffer), 
				(struct sockaddr *) NULL, 0, &s_sndrcvinfo, 0);
	}

	close(sock_listen);
	return 0;
}

