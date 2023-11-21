CC = gcc
CFLAGS = 

all: nm client ss

nm: naming-server.c
	$(CC) $(CFLAGS) naming-server.c hashing.c data_utils.c network_wrapper.c nm_utils.c print_utils.c search_tree.c -o nm

client: client.c
	$(CC) $(CFLAGS) client.c print_utils.c -o client

ss: storage-server.c
	$(CC) $(CFLAGS) storage-server.c scan_directory.c ss_utils.c network_wrapper.c print_utils.c data_utils.c -o ss

clean:
	rm -f nm client ss
