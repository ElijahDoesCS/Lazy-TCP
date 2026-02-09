#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief return the contents of a file based on its file path
 * @param path is the path of the file
 * @param message_len is a pointer to the message so we can keep track of it
 */
char *http_serve_file(const char *filepath, int *message_len);

/**
 * @brief If the user sends a valid request for our hello world, respond with a message
 * @param request_len is the length of the request
 * @param message_len is a pointer to the message len so we can keep track of it
 */
char *http_handle_request(char *request, int reqest_len, int *message_len);

#endif