#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTBUFSZ PIPE_BUF

// Return 1, 0 otherwise when we are ready to flush the buffer.
int
feed(char c, int *noutbuf, char outbuf[])
{
	if (c == '\n') {
		outbuf[(*noutbuf)++] = c;
		return 1;
	}

	if (*noutbuf == OUTBUFSZ-1) {
		return 0;
	}

	outbuf[(*noutbuf)++] = c;
	return 0;
}

int
main(int argc, char **argv) {
	char *prefix;
	char  outbuf[OUTBUFSZ];
	int   noutbuf = 0;

	if (argc != 2) {
		fprintf(stderr, "%s PREFIX\n", argv[0]);
		exit(1);
	}
	prefix = argv[1];

	int prefixlen = strlen(prefix);
	int truncprefixlen = (prefixlen > 510 ? 510 : prefixlen);
	// We must have enough space for the prefix, one char and a new line.
	assert(truncprefixlen + 2 <= sizeof(outbuf));
	memcpy(outbuf, prefix, truncprefixlen);
	noutbuf += truncprefixlen;
	
	char inbuf[PIPE_BUF];
	
	while (1) {
		int nread = read(STDIN_FILENO, inbuf, sizeof(inbuf));

		if (nread < 0) {
			perror("error reading data");
			exit(1);
		}
		if (nread == 0) {
			break;
		}

		for (int i = 0; i < nread; i++) {
			if (feed(inbuf[i], &noutbuf, outbuf)) {
				if (write(STDOUT_FILENO, outbuf, noutbuf) < 0) {
					perror("error writing output");
					exit(1);
				}
				noutbuf = truncprefixlen;
			}
		}
	}
	
	if(!feed('\n', &noutbuf, outbuf)) {
		assert(0 || "unexpected error");
	}

	if (noutbuf > truncprefixlen + 1) {
		if (write(STDOUT_FILENO, outbuf, noutbuf) < 0) {
			perror("error writing output");
			exit(1);
		}
	}

	return 0;
}