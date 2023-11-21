#ifndef _ERRORS_H
#define _ERRORS_H

// Define error codes
#define ERR_FILE_NOT_FOUND 1
#define ERR_FILE_IN_USE 2
#define ERR_PERMISSION_DENIED 3
#define ERR_INVALID_PATH 4
#define ERR_INVALID_COMMAND 5  // invalidPath
#define ERR_DISPLAY 6          // errorDisplay
#define ERR_CANNOT_OPEN_FILE 7 // custom_read
#define ERR_CANNOT_DELETE 8    // custom_delete
#define ERR_DATA_RECEIVE 9     // in client
#define ERR_NO_HEARTBEAT 10
#define ERR_DATA_SEND 11
#define ERR_MEMORY_FAILURE 12
#define ERR_SOCKET_FAILURE 13
#define ERR_BIND_FAILURE 14
#define ERR_LISTENING 15
#define ERR_ACCEPTING 16
#define ERR_INVALID_TYPE 17

#endif