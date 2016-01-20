#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define content "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut ac consequat est. Mauris ut tempor erat, sit amet suscipit felis. Nullam ut neque quis sapien vestibulum pellentesque. Maecenas mattis libero ullamcorper arcu malesuada feugiat. Fusce scelerisque a diam eget scelerisque. Cras pharetra mauris at libero maximus, vel consequat lacus vulputate. In non mauris id est tempor aliquet dapibus eget mi. Proin molestie auctor lacus, sit amet vehicula erat tincidunt nec. Fusce nec sapien tincidunt, pharetra sapien eget, interdum magna. Nam vehicula neque nisl, eget tempus elit commodo ac. Aliquam eget tempor ligula. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin eu vestibulum metus. Duis ornare nisl dui, vehicula fringilla tortor lobortis ac. In porta eget nunc vitae scelerisque. Cras nisi tellus, vehicula eget rutrum et, viverra eu massa. Fusce sagittis dolor enim. Nulla pretium arcu risus, eu gravida lacus vulputate et. Aenean laoreet felis aliquet mauris laoreet, et mollis enim mollis. Morbi in neque nec quam ornare tempor eu id risus. Aliquam venenatis, augue at feugiat consequat, libero ligula fermentum arcu, sed tempus lectus eros at sapien. Vivamus porttitor dolor sit amet eros faucibus vestibulum. Vestibulum augue dolor, dictum eget ipsum et, mollis ultricies enim. Nam convallis venenatis ex, vel viverra dolor congue et. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin vel bibendum elit. Fusce molestie vel tellus id porttitor. Ut aliquam, diam vitae rutrum varius, nunc massa pellentesque ligula, eu mattis urna metus at ipsum. Duis a laoreet ante, iaculis iaculis tortor. Vivamus volutpat justo lacinia est elementum tempus. Sed id justo lacus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Integer sodales placerat est tincidunt tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus auctor nisl tellus, id hendrerit ligula consequat at. Duis nunc sapien, sagittis in lobortis eget, fringilla sit amet magna. Etiam rutrum, nisi rutrum ullamcorper sagittis, mi elit feugiat purus, at finibus erat arcu in sem. Maecenas vel sapien et quam fermentum vulputate id quis est. In eget leo porttitor turpis auctor efficitur. Cras nec semper urna, eu faucibus elit. Morbi nec enim diam. In ut placerat eros. Pellentesque porta id tellus in efficitur. Integer dignissim purus vel lorem pretium accumsan. Nullam in erat mauris. Maecenas urna arcu, ultrices et urna non, hendrerit sagittis tortor. Nam nec lacus neque. Sed risus nibh, blandit vel tincidunt sed, scelerisque eu est. Sed sodales rhoncus diam, vitae malesuada libero eleifend ac. Sed nulla sem, aliquam id urna eget, efficitur consectetur velit. Sed id erat et lorem fermentum laoreet. Sed mollis lectus at velit vestibulum condimentum. Suspendisse hendrerit diam eu velit hendrerit mattis. Cras a neque nec urna gravida porta sed vitae nibh. Phasellus in commodo magna. Maecenas libero sem, auctor ut tristique non, congue vitae nibh. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin porttitor mi non condimentum sagittis. In suscipit sem ut justo viverra volutpat. Curabitur porta tincidunt nulla posuere finibus. Phasellus turpis leo, hendrerit sed est a, eleifend luctus nisi. Quisque quis porttitor leo. Nunc hendrerit mattis aliquam. Donec ultricies sodales euismod."

void echo_main(int sock, int num_streams, int characters_to_send) {
	int stat, stream_i, char_i;
	int content_size = sizeof(content);
	int buffer_size = 1024;
	char buffer[buffer_size];

    // Loop over number Stream Identifier numbers
	for (stream_i = 0; stream_i < num_streams; stream_i++) {

        // Loop over characters to send
		for (char_i = 0; char_i < characters_to_send; char_i++) {

			if ((char_i == characters_to_send - 1 && characters_to_send < buffer_size ) 
                    || (char_i % buffer_size == 0 && char_i > 0)) {
                        
                // The buffer is full, or we put in the required number of characters, so send
				stat = sctp_sendmsg(sock, buffer, (size_t) strlen(buffer), NULL, 0, 0, 0, stream_i, 0, 0);
				// Debug print
                printf("chars=%i, streams=%i, stream_i=%i, char_i=%i, stat=%i\n", 
                    characters_to_send, num_streams, stream_i, char_i, stat);
			}
			buffer[char_i % buffer_size] = content[char_i % content_size];
		}
	}
}

int main(int argc, char **argv) {
	int sock_listen, sock_server, stat;
	struct sockaddr_in server_addr;
	struct sctp_initmsg s_initmsg;
	int echo_port;
	int i = 0;

	echo_port = 3007;

	sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(echo_port);

	stat = bind(sock_listen, (struct sockaddr *) &server_addr, sizeof(server_addr));

	// SCTP parameter 	// TODO: These should be sent as function parameters probably
	memset(&s_initmsg, 0, sizeof(s_initmsg));
	s_initmsg.sinit_num_ostreams = 10;
	s_initmsg.sinit_max_instreams = 10;
	s_initmsg.sinit_max_attempts = 5;

	stat = setsockopt(sock_listen, IPPROTO_SCTP, SCTP_INITMSG, &s_initmsg, sizeof(s_initmsg));
	if (stat < 0) {
		perror("Socket Option error");
		exit(-1);
	}

	listen(sock_listen, 5);
    
	while (1) {
		printf("SCTP server accepting\n");
		sock_server = accept(sock_listen, (struct sockaddr *) NULL, (socklen_t *) NULL);
		if (sock_server == -1) {
			perror("accept");
			exit(-1);
		}
		echo_main(sock_server, 4, 128);
	}

	close(sock_listen);
	return 0;
}

