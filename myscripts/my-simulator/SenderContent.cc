#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>

#include "SenderContent.h"

using namespace std;

SenderContent::SenderContent(int length) {
	len = length;
	i = 0;
}

int SenderContent::fill(char* buffer, int buffer_size) {
	while (i < len) {
		int content_i = i % (CONTENT_SIZE - 1);
		int buffer_i = i % (buffer_size - 1);
		buffer[buffer_i] = CONTENT[content_i];
		i++;
		if ((i == len) || (buffer_i + 1 == buffer_size - 1)) {
			buffer[buffer_i + 1] = '\0';
			return 1;
		}
	}
	return 0;
}
