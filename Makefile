
all: alt

alt: alt.c
	$(CC) -g -Wall -Wextra -O2 -o $@ $<

clean:
	rm -f alt
