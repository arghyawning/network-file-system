#include "../include/common.h"

void print_error_message(int error_code)
{
    switch (error_code)
    {
    case ERR_FILE_NOT_FOUND:
        printf("Error: File not found\n");
        break;
    case ERR_FILE_IN_USE:
        printf("Error: File in use by another client\n");
        break;
    case ERR_PERMISSION_DENIED:
        printf("Error: Permission denied\n");
        break;
    case ERR_INVALID_PATH:
        printf("Error: Invalid path\n");
        break;
    case ERR_INVALID_COMMAND:
        printf("Error: Invalid command\n");
        break;
    case ERR_DISPLAY:
        printf("Error: Display error\n");
        break;
    case ERR_CANNOT_OPEN_FILE:
        printf("Error: Cannot open file\n");
        break;
    case ERR_CANNOT_DELETE:
        printf("Error: Cannot delete file\n");
        break;
    case ERR_DATA_RECEIVE:
        printf("Error: Cannot receive data\n");
        break;
    case ERR_NO_HEARTBEAT:
        printf("Error: No heartbeat\n");
        break;
    case ERR_DATA_SEND:
        printf("Error: Cannot send data\n");
        break;
    case ERR_MEMORY_FAILURE:
        printf("Error: Memory failure\n");
        break;
    case ERR_SOCKET_FAILURE:
        printf("Error: Socket failure\n");
        break;
    case ERR_BIND_FAILURE:
        printf("Error: Bind failure\n");
        break;
    case ERR_LISTENING:
        printf("Error: Listening error\n");
        break;
    case ERR_ACCEPTING:
        printf("Error: Accepting error\n");
        break;
    case ERR_INVALID_TYPE:
        printf("Error: Invalid type\n");
        break;
    default:
        printf("Unknown error code: %d\n", error_code);
    }
}