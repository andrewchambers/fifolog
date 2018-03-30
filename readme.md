# fifolog

when run as ```fifolog PREFIX``` fifolog reads data from stdin, then outputs lines in the format:

PREFIX || LINE[0..(PIPE_BUF-LEN(PREFIX))]

truncating lines longer than PIPE_BUF. If PREFIX is longer than 240 characters it is truncated to 240 characters.

This enables atomic labeled writes to a unix pipe. Consider
the following example:

```
mkfifo logstream
(cat data | fifolog data-1: > logstream)  &
(cat data | fifolog data-2: > logstream)  &
(cat data | fifolog data-3: > logstream)  &
cat logstream
```

The terminal will get non interleaved labeled line output guaranteed because each line written to fifo is smaller than PIPE_BUF.