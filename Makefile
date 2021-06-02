CC=gcc
CFLAGS="-Wall"

debug:clean
	$(CC) $(CFLAGS) -g -o friendlymakegenerator main.c
stable:clean
	$(CC) $(CFLAGS) -o friendlymakegenerator main.c
clean:
	rm -vfr *~ friendlymakegenerator
