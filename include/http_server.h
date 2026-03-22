#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// HTTP server port
#define HTTP_PORT 80

// Initialize HTTP server
void http_server_init(void);

// Start HTTP server
void http_server_start(void);

// Stop HTTP server
void http_server_stop(void);

// Check if server is running
bool http_server_is_running(void);

// Process HTTP requests (called from main loop)
void http_server_poll(void);

// Set configuration pointer
void http_server_set_config(config_t *config);

#endif // HTTP_SERVER_H
