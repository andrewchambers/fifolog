#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
usage(char *name)
{
    fprintf(stderr, "%s PREFIX\n", name);
}

int
main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        exit(1);
    }
    int eof = 0;
    char buf[PIPE_BUF] = {0};
    ssize_t prefixlen = strlen(argv[1]);
    prefixlen = (prefixlen > 1000) ? 1000 : prefixlen;
    memcpy(buf, argv[1], prefixlen);

    char *inbuf = buf + prefixlen;
    ssize_t bufsz = sizeof(buf) - prefixlen;
    ssize_t nbuff = 0;
    
    while (!eof) {
        assert(nbuff <= bufsz);

        int i = 0;
        int hasnl = 0;
        ssize_t nread = 0;
        ssize_t scan = 0;
        ssize_t nafternl = 0;

        // Read at most PIPE_BUF bytes, or until newline

        while (nbuff < bufsz) {
            nread = read(STDIN_FILENO, inbuf + nbuff, bufsz - nbuff);
 
            if (nread < 0) {
                perror("error reading data");
                exit(1);
            }

            nbuff += nread;
            assert(nbuff <= bufsz);

            for (; scan < nbuff; scan++) {
                if (inbuf[scan] == '\n') {
                    hasnl = 1;
                    goto print_line;
                }
            }

            if (nread == 0) {
                eof = 1;
                goto print_line;
            }
        }

        print_line:

        assert(nbuff <= bufsz);
        assert((hasnl && inbuf[scan] == '\n')
          || (!hasnl && nbuff <= bufsz && nbuff == scan));
        
        if (write(STDOUT_FILENO, buf, prefixlen + (hasnl ? scan + 1 : nbuff)) < 0) {
            perror("error writing output");
            exit(1);
        }

        if (!hasnl) {
            // truncate the line by just reading until '\n', EOF, or error.
            char throwaway[1] = {0};
            while(throwaway[0] != '\n') {
                nread = read(STDIN_FILENO, throwaway, sizeof(throwaway));
                if (nread < 0) {
                    perror("error reading data");
                    exit(1);
                }
                if (nread == 0) {
                    break;
                }
            }

            throwaway[0] = '\n';
            if (write(STDOUT_FILENO, throwaway, sizeof(throwaway)) < 0) {
                perror("error writing output");
                exit(1);
            }
        }

        // Reset nbuff and the buffer to deal with the next line.
        nafternl = hasnl ? (nbuff - (scan + 1)) : 0;

        for (i = 0; i < nafternl; i++) {
            inbuf[i] = inbuf[scan + 1 + i];
        }

        nbuff = nafternl;
    }


    return 0;
}