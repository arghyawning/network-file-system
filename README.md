# Network File System from scratch

Course Project for the course CS3.301: **Operating Systems and Networks**

Monsoon 2023, IIIT Hyderabad.

## Team

2021113013 - [Radhikesh Agrawal](mailto:radhikesh.agrawal@research.iiit.ac.in) <br>
2021114008 - [Harinie Sivaramasethu](mailto:harinie.s@research.iiit.ac.in) <br>
2021115008 - [Arghya Roy](mailto:arghya.roy@research.iiit.ac.in) <br>

## Instructions to run

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
- `src/`: Contains all the source files
  - `client.c`:
  - `naming-server.c`:
  - `storage-server.c`:
- `Makefile`: For compiling the code
- `shell.c`: This is for initialisation, inputs, parsing, and calling the appropriate functions.

## Assumptions

- Each storage server has max 100 paths with each of max length 50

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

---

### Resources used

#### (kind of redundant since we don't need to implement a FS from scratch. It is much easier than what we were expecting)

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

#### Miscelleneous Implementation Specific Resources

- https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
- https://github.com/nikhilroxtomar/TCP-Client-Server-Implementation-in-C

### Challenges faced

- We misunderstood the assignment initially, thinking we have to create a distributed file system from scratch
- Figuring out how to `git push` things simultaneously
- Completing it in such a short period of time among other coursework
