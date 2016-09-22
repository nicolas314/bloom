
CFLAGS = -g -I. -Wall -pedantic $(EXTRA)

all: bloom

bloom: bloom.c bloom.h
	$(CC) $(CFLAGS) -o $@ $< -DMAIN -lm

clean:
	rm -f bloom
	find . \( -name \*.dSYM  -prune \) -exec rm -rf '{}' ';'
