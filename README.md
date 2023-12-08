# Network File System from scratch

Course Project for the course CS3.301: **Operating Systems and Networks**

Monsoon 2023, IIIT Hyderabad.
## Table of Contents

- [Network File System from scratch](#network-file-system-from-scratch)
  - [Table of Contents](#table-of-contents)
  - [Team](#team)
  - [Instructions to run](#instructions-to-run)
  - [Functionalities](#functionalities)
    - [read](#read)
    - [write](#write)
    - [delete](#delete)
    - [copy](#copy)
    - [create](#create)
    - [file information](#file-information)
  - [File Structure](#file-structure)
  - [Assumptions](#assumptions)
  - [NAMING SERVER](#naming-server)
  - [STORAGE SERVER](#storage-server)
  - [CLIENT](#client)
  - [MULTIPLE CLIENTS](#multiple-clients)
    - [Concurrent Client Access:](#concurrent-client-access)
    - [Concurrent File Reading:](#concurrent-file-reading)
  - [ERROR CODES](#error-codes)
  - [SEARCH IN NAMING SERVERS](#search-in-naming-servers)
  - [REDUNDANCY/REPLICATION:](#redundancyreplication)
  - [Caching \& Logging](#caching--logging)
    - [Resources used for Caching \& Logging](#resources-used-for-caching--logging)
    - [Caching](#caching)
    - [Logging](#logging)
  - [Resources used](#resources-used)
    - [Miscelleneous Implementation Specific Resources](#miscelleneous-implementation-specific-resources)
  - [Challenges faced](#challenges-faced)

## Team

2021113013 - [Radhikesh Agrawal](mailto:radhikesh.agrawal@research.iiit.ac.in) <br>
2021114008 - [Harinie Sivaramasethu](mailto:harinie.s@research.iiit.ac.in) <br>
2021115008 - [Arghya Roy](mailto:arghya.roy@research.iiit.ac.in) <br>

## Instructions to run
```bash
cd src
MAKE
```

## Functionalities

### read

Input format:

```bash
READ <filepath>
```

### write

Input format:

```bash
WRITE <filepath>
```

### delete

Input format:

```bash
DELETE <filepath>
```

### copy

Input format:

```bash
COPY <source_filepath> <destination_path>
```

### create

Input format:

```bash
CREATE <filepath>
```

### file information

Input format:

```bash
FILEINFO <filepath>
```

## File Structure

- `include/`: Contains all the header files
  - `colours.h`  
  - `common.h` : includes libraries, constants, function declarations, data structures 
  - `errors.h`: includes error codes
  - `hashing.h`
  - `naming_server.h`
  - `shell.h`:
  - `ss.h`
  - `utils.h`

- `src/`: Contains all the source files
  - `client.c`
  - `naming-server.c`
  - `storage-server.c`
  - `data_utils.c`
  - `errors.c` : For error code handling
  - `hashing.c`
  - `network_wrapper.c`
  - `nm_utils.c`
  - `print_utils.c`
  - `scan_directory.c`
  - `ss_utils.c`
  - `utils.c`
  - `Makefile` : For compiling the code


- `shell.c`: This is for initialisation, inputs, parsing, and calling the appropriate functions.

## Assumptions
- Abbreviations:  
  - ss : storage server


- We define the following ports:   

| PORTS                         | Value |
| :---------------------------- | ----: |
| LEADER SERVER PORT FOR CLIENT |  8000 |
| LEADER SERVER PORT            |  8080 |
| SERVER BACKLOG                |    10 |
| BASE PORT FOR STORAGE SERVER  |  5000 |
| BASE PORT FOR CLIENT          |  6000 |


- We define the below constraints  

| Constraint             | Value |
| :--------------------- | ----: |
| MAX PORTS PER SS       |     4 |
| MAX PORTS PER CLIENT   |     3 |
| MAX CONNECTION TRIES   |     4 |
| READ SIZE              |  1024 |
| SERIALIZED BUFFER SIZE |  8092 |
| MAX PATH SIZE          |   256 |
| MAX BUFFER SIZE        |  1024 |
| MAX USERNAME SIZE      |    32 |
| MAX COMMAND NAME       |    16 |
| MAX INPUT LENGTH       |   266 |
| MAX COMMAND ARGUMENTS  |     2 |
| MAX CHUNK SIZE         |    16 |
| MAX FILES              |    32 |
| MAX NEW DIRECTORIES    |     8 |
| MAX SS                 |    16 |
| MAX CLIENTS            |    16 |
| MAX REDUNDANT SS       |     2 |
| HEARTBEAT PERIOD       |     2 |
| RETRY INTERVAL         |     1 |

<br>

## NAMING SERVER
- Naming Server maintains information about connected Storage Servers, clients, and file structures.
  
- The code utilizes hash tables (fileshash and dirhash) for efficient file and directory search operations.
- There is a function (storage_server_heartbeat) which is responsible for managing the heartbeat mechanism between the Naming Server and Storage Servers. The heartbeat mechanism ensures the continuous monitoring of Storage Servers' availability.

- first_connection_ss function manages the initial connection from Storage Servers to the Naming Server. It registers new Storage Servers, assigns ports, and initiates heartbeat threads and also handles disconnection events from Storage Servers.
  
- first_connection_client handles the initial connection from Clients to the Naming Server. It registers new Clients, assigns ports, and creates threads for further communication and handles disconnection events from Clients.
- Multithreading is used to handle concurrent connections from multiple Storage Servers and Clients.


## STORAGE SERVER
- We have structure `StorageServerInfo` which is used to represent information about the storage server and structure `CombinedFilesInfo` which is used for combined information about files and directories in the storage server. Mutex is also used for synchronization.
  
- Utility functions are implemented: removeDirectoryRecursive, readFile, readFileInfo, writeToFile, handle_ctrl_c
  
- Storage server is initialized with a connection to the leader server using predefined IP address and port. Information about files and directories are also initialized in the storage server.
  
- There is another heartbeat thread which listens for incoming connections from the naming server. This receives and sends heartbeat messages between the storage server and the naming server.
  
- `namingServerConnectionThread` is created to accept, recieve and execute commands (e.g., CREATE, DELETE) from the naming server. `clientConnectionThread` is made to accept and recieve client requests and to perform the corresponding actions such as READ, WRITE, etc. 

- Mainly, the code initiates the storage server and creates three threads: heartbeat, naming server connection, and client connection.

- The program utilizes pthreads to achieve concurrent execution of tasks, including heartbeat handling, naming server communication, and client interactions.

- The storage server interacts with clients by reading and writing files, providing file information, and handling various client requests. The communication with clients involves different threads to handle multiple client connections concurrently.


<br>

## CLIENT
- Our code handles client requests for reading, writing, and retrieving information, etc about files. The naming_server_connection function initiates communication with the Naming Server (NM). The NM sends relevant information about the Storage Server (SS) to the client based on the requested file path.
- CREATE, DELETE, COPY, FILEINFO, READ, WRITE commands have been implemented.

<br>


## MULTIPLE CLIENTS
### Concurrent Client Access:
- `client_connection` is a function that handles incoming client connections. It creates a thread for each connected client, allowing multiple clients to be served simultaneously.
- The code for this can be found in `src/naming-server.c`.

### Concurrent File Reading:
- The code handles concurrent file reading by allowing multiple clients to read the same file simultaneously. This is done by checking the command type in the `client_request` and performing action (READ).
- The code for this can be found in `src/naming-server.c` and `src/client.c`

<br>

## ERROR CODES
- A set of error codes have been defined and implemented across the whole program. It handles errors such as display, usage of file, memory, socket and bind failures, data errors, etc.
- The code for this can be found in `included/errors.h` and `src/errors.c`.

<br>

## SEARCH IN NAMING SERVERS
- A hash table is implemented for efficient search and retrieval of directory and file information. It is implemented using two structures, bucket for files and bucket_dir for directories.
- The code for this can be found in `src/hashing.c` and `include/hashing.h`.

<br>

- A directory tree data structure is also implemented. Operations on this structure include filling the tree with information about directories and files on different storage servers, searching for a specific file in the tree, and finding the closest directory for a given file.
- Code for this can be found in `src/search-tree.c`.

<br>

## REDUNDANCY/REPLICATION:
- We have an `ss_connection_status` array to track the connection status of each Storage Server. Each element represents a Storage Server's connection status (0 for not connected, 1 for connected).
  
- When a Storage Server connects or disconnects, the naming server updates the ss_connection_status array accordingly.

- We have a function `makeRedundantServers` which assigns redundant servers for a given storage server. It starts by checking the number of active storage servers (except the main server).
  
- We define a MAX_REDUNDANT_SS which is used to determine max number of redundant servers to assign.
  
- The logic involves selecting random active storage servers (except the main server) to act as redundant servers. 
  
- The code for this can be found in `src/naming-server.c`.

<br>

## Caching & Logging

### Resources used for Caching & Logging

- [Caching in Large-Scale Distributed File Systems](http://csis.pace.edu/~marchese/CS865/Papers/blaze_397.pdf)
- [IBM CacheFS](https://www.ibm.com/docs/en/aix/7.2?topic=performance-cache-file-system)

### Caching

- Will have a thread which periodically checks the consistency of files stored in the cache. It does this by comparing the current modification time to the previous modification time.
- If the modification times are different, all data and attributes for the directory or file are purged from the cache, and new data and attributes are retrieved from the back file system.

### Logging

We will do logging separately for Storage-Servers, Naming-Server and Client. We will log the data in the following format:

$\texttt{timestamp | execution-time | server | client | command-name | arguments | reply-data}$

where

- $\texttt{timestamp}$: time the reply message was received
- $\texttt{execution-time}$: time (in $\mu$s) that elapsed between the call and reply
- $\texttt{server}$: name or ip address of the server
- $\texttt{client}$: name or ip address of the client followed by the userid that issued the client
- $\texttt{command-name}$: name of the program invoked (like `READ`, `WRITE`, etc)
- $\texttt{arguments}$: command dependent arguments
- $\texttt{reply-data}$: command dependent reply-data

Example:

$\texttt{2690529992.167140 | 11717 | paramount | merckx.321 | read | \{"7b1£00000000083c", 0, 8192\} | ok, 1871}$

Explanation:

In this example, $\texttt{uid 321}$ at client $\texttt{"merckx"}$ issued an NFS read command to server $\texttt{"paramount"}$. The reply was issued at (Unix time) $\texttt{690529992.167140}$ seconds; the call command occurred $\texttt{11717 microseconds}$ earlier. Three arguments are logged for the read call: the file handle from which to read (represented as a hexadecimal string), the offset from the beginning of the file, and the number of bytes to read. In this example, $\texttt{8192}$ bytes are requested starting at the beginning (byte 0) of the file whose handle is $\texttt{"7b1£00000000083c"}$. The command completed successfully (status "ok"),
and 1871 bytes were returned. Of course, the reply message also included the 1871 bytes of data from the file, but that field of the reply is not logged.

<br>

## Resources used

__(kind of redundant since we don't need to implement a FS from scratch. It is much easier than what we were expecting)__

- MIT Lectures on Distributed systems
- Chinese guy telling about DFS (nice video. Do watch) - [How to Design Distributed File System](https://www.youtube.com/watch?v=g6VWTEtUsQY)
- https://book.mixu.net/distsys/
- https://martinfowler.com/articles/patterns-of-distributed-systems/
- [FatFs](http://elm-chan.org/fsw/ff/00index_e.html)
- [Writing FS for Fun](https://github.com/cppdug/presentations/blob/master/files/2018-05-14/WritingFS4Fun.pptx)
- [OS8](https://codex.cs.yale.edu/avi/os-book/OS8/os8c/slide-dir/index.html)
- [Writing FS from scratch in RUST](https://blog.carlosgaldino.com/writing-a-file-system-from-scratch-in-rust.html)
- [Understanding multi level indexing for iNodes](https://www.cs.utexas.edu/~lorenzo/corsi/cs372/06F/hw/11sol.html)
- https://book.mixu.net/distsys/
- [Some Nice FS ideas](https://martinfowler.com/articles/patterns-of-distributed-systems/)

### Miscelleneous Implementation Specific Resources

- https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
- https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C


<br>

## Challenges faced

- We misunderstood the assignment initially, thinking we have to create a distributed file system from scratch
- Figuring out how to `git push` things simultaneously
- Completing it in such a short period of time among other coursework
  