#ifndef WEB_UI_H
#define WEB_UI_H

#include <stddef.h>
#include <stdint.h>

// HTML page types
typedef enum {
    PAGE_DASHBOARD,
    PAGE_SETTINGS,
    PAGE_BLOCKLIST,
    PAGE_LOGS,
    PAGE_API_STATUS,
    PAGE_API_CONFIG,
    PAGE_API_BLOCKLIST,
    PAGE_API_LOGS
} page_type_t;

// Get HTML content for a page
const char* web_ui_get_page(page_type_t page, size_t *length);

// Get content type for a page
const char* web_ui_get_content_type(page_type_t page);

// Process form data
bool web_ui_process_form(const char *form_data, size_t length, char *key, size_t key_size, 
                         char *value, size_t value_size);

// Generate JSON response for status
void web_ui_generate_status_json(char *buffer, size_t buffer_size);

// Generate JSON response for config
void web_ui_generate_config_json(char *buffer, size_t buffer_size);

// Generate JSON response for blocklist
void web_ui_generate_blocklist_json(char *buffer, size_t buffer_size);

// Generate JSON response for logs
void web_ui_generate_logs_json(char *buffer, size_t buffer_size);

#endif // WEB_UI_H
