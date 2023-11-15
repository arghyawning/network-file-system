# Network File System from scratch

Course Project for the course CS3.301: <b>Operating Systems and Networks</b>
<br> Monsoon 2023, IIIT Hyderabad.

#### Team

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

### Challenges faced

- We misunderstood the assignment initially, thinking we have to create a distributed file system from scratch
- Figuring out how to `git push` things simultaneously
- Completing it in such a short period of time among other coursework
