#ifndef _SENDERCONTENT_H
#define _SENDERCONTENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>        /* for memset */
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>

//~ #define CONTENT "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut ac consequat est. Mauris ut tempor erat, sit amet suscipit felis. Nullam ut neque quis sapien vestibulum pellentesque. Maecenas mattis libero ullamcorper arcu malesuada feugiat. Fusce scelerisque a diam eget scelerisque. Cras pharetra mauris at libero maximus, vel consequat lacus vulputate. In non mauris id est tempor aliquet dapibus eget mi. Proin molestie auctor lacus, sit amet vehicula erat tincidunt nec. Fusce nec sapien tincidunt, pharetra sapien eget, interdum magna. Nam vehicula neque nisl, eget tempus elit commodo ac. Aliquam eget tempor ligula. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin eu vestibulum metus. Duis ornare nisl dui, vehicula fringilla tortor lobortis ac. In porta eget nunc vitae scelerisque. Cras nisi tellus, vehicula eget rutrum et, viverra eu massa. Fusce sagittis dolor enim. Nulla pretium arcu risus, eu gravida lacus vulputate et. Aenean laoreet felis aliquet mauris laoreet, et mollis enim mollis. Morbi in neque nec quam ornare tempor eu id risus. Aliquam venenatis, augue at feugiat consequat, libero ligula fermentum arcu, sed tempus lectus eros at sapien. Vivamus porttitor dolor sit amet eros faucibus vestibulum. Vestibulum augue dolor, dictum eget ipsum et, mollis ultricies enim. Nam convallis venenatis ex, vel viverra dolor congue et. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin vel bibendum elit. Fusce molestie vel tellus id porttitor. Ut aliquam, diam vitae rutrum varius, nunc massa pellentesque ligula, eu mattis urna metus at ipsum. Duis a laoreet ante, iaculis iaculis tortor. Vivamus volutpat justo lacinia est elementum tempus. Sed id justo lacus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Integer sodales placerat est tincidunt tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus auctor nisl tellus, id hendrerit ligula consequat at. Duis nunc sapien, sagittis in lobortis eget, fringilla sit amet magna. Etiam rutrum, nisi rutrum ullamcorper sagittis, mi elit feugiat purus, at finibus erat arcu in sem. Maecenas vel sapien et quam fermentum vulputate id quis est. In eget leo porttitor turpis auctor efficitur. Cras nec semper urna, eu faucibus elit. Morbi nec enim diam. In ut placerat eros. Pellentesque porta id tellus in efficitur. Integer dignissim purus vel lorem pretium accumsan. Nullam in erat mauris. Maecenas urna arcu, ultrices et urna non, hendrerit sagittis tortor. Nam nec lacus neque. Sed risus nibh, blandit vel tincidunt sed, scelerisque eu est. Sed sodales rhoncus diam, vitae malesuada libero eleifend ac. Sed nulla sem, aliquam id urna eget, efficitur consectetur velit. Sed id erat et lorem fermentum laoreet. Sed mollis lectus at velit vestibulum condimentum. Suspendisse hendrerit diam eu velit hendrerit mattis. Cras a neque nec urna gravida porta sed vitae nibh. Phasellus in commodo magna. Maecenas libero sem, auctor ut tristique non, congue vitae nibh. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin porttitor mi non condimentum sagittis. In suscipit sem ut justo viverra volutpat. Curabitur porta tincidunt nulla posuere finibus. Phasellus turpis leo, hendrerit sed est a, eleifend luctus nisi. Quisque quis porttitor leo. Nunc hendrerit mattis aliquam. Donec ultricies sodales euismod. "
#define CONTENT "abcdefghijklmnopqrstuvwxyz"
#define CONTENT_SIZE sizeof(CONTENT)

using namespace std;

class SenderContent {
public:
	/**
	 * Create a new text generating object with specified length.
	 * @param length The length of the text in bytes.
	 */
	SenderContent(int length);
	/**
	 * Fill the buffer with characters from the Lorem Ipsum string.
	 * @param buffer The buffer to write text into.
	 * @param buffer_size Size of buffer in bytes.
	 * @return Returns 1 if there are more characters left, 0 if no text remain.
	 */
	int fill(char* buffer, int buffer_size);

private:
	int len, i;
};

#endif /* _SENDERCONTENT_H */
