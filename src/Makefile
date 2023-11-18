CC = gcc
CFLAGS = 

all: nm client ss

nm: naming-server.c
	$(CC) $(CFLAGS) naming-server.c -o nm

client: client.c
	$(CC) $(CFLAGS) client.c -o client

ss: storage-server.c
	$(CC) $(CFLAGS) storage-server.c scan_directory.c -o ss

clean:
	rm -f nm client ss