# fifolog

when run as ```fifolog PREFIX``` fifolog reads '\n' delimited lines from stdin, then outputs lines in the format:

PREFIX || LINE[0..MIN(LEN(LINE) - 1, PIPE_BUF - LEN(PREFIX) - 1)] || '\n'

If PREFIX is longer than 510 characters it is truncated to 510 characters.

fifolog terminates with a non zero exit code if any IO errors are encountered.

This enables atomic labeled writes to a unix pipe. Consider
the following example:

```
mkfifo logstream
(cat data | fifolog data-1: > logstream)  &
(cat data | fifolog data-2: > logstream)  &
(cat data | fifolog data-3: > logstream)  &
cat logstream
```

The terminal will get interleaved, but not jumbled labeled line output guaranteed because each labeled write to fifo is smaller than PIPE_BUF.
