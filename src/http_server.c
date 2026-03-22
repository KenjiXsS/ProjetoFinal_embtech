#include "http_server.h"
#include "web_ui.h"
#include "firewall.h"
#include "network_monitor.h"
#include "system_monitor.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

#define HTTP_BUFFER_SIZE 2048

static struct tcp_pcb *server_pcb = NULL;
static bool server_running = false;
static config_t *g_config = NULL;

// HTTP Response headers
static const char *html_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
static const char *json_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
static const char *cors_header = "HTTP/1.1 204 No Content\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n\r\n";

// Parse HTTP request
static int parse_request(const char *request, char *method, size_t method_size,
                         char *path, size_t path_size, char *body, size_t body_size) {
    if (!request || strlen(request) < 10) return -1;
    
    // Extract method
    const char *space = strchr(request, ' ');
    if (!space) return -1;
    
    size_t method_len = space - request;
    if (method_len >= method_size) method_len = method_size - 1;
    strncpy(method, request, method_len);
    method[method_len] = '\0';
    
    // Extract path
    const char *path_start = space + 1;
    const char *path_end = strchr(path_start, ' ');
    if (!path_end) path_end = path_start + strlen(path_start);
    
    size_t path_len = path_end - path_start;
    if (path_len >= path_size) path_len = path_size - 1;
    strncpy(path, path_start, path_len);
    path[path_len] = '\0';
    
    // Extract body (if exists)
    if (body && body_size > 0) {
        const char *body_start = strstr(request, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            strncpy(body, body_start, body_size - 1);
            body[body_size - 1] = '\0';
        } else {
            body[0] = '\0';
        }
    }
    
    return 0;
}

// Send HTTP response
static err_t send_response(struct tcp_pcb *tpcb, const char *header, const char *body) {
    if (tcp_sndbuf(tpcb) < strlen(header) + strlen(body) + 10) {
        return ERR_MEM;
    }
    
    tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    if (body && strlen(body) > 0) {
        tcp_write(tpcb, body, strlen(body), TCP_WRITE_FLAG_COPY);
    }
    tcp_output(tpcb);
    
    return ERR_OK;
}

// Handle API requests
static err_t handle_api_request(struct tcp_pcb *tpcb, const char *method, const char *path, const char *body) {
    char response[HTTP_BUFFER_SIZE];
    const char *header = json_header;
    
    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        return send_response(tpcb, cors_header, "");
    }
    
    // GET /api/status
    if (strcmp(path, "/api/status") == 0 && strcmp(method, "GET") == 0) {
        web_ui_generate_status_json(response, sizeof(response));
        return send_response(tpcb, header, response);
    }
    
    // GET /api/blocklist
    if (strcmp(path, "/api/blocklist") == 0 && strcmp(method, "GET") == 0) {
        web_ui_generate_blocklist_json(response, sizeof(response));
        return send_response(tpcb, header, response);
    }
    
    // GET /api/logs
    if (strcmp(path, "/api/logs") == 0 && strcmp(method, "GET") == 0) {
        web_ui_generate_logs_json(response, sizeof(response));
        return send_response(tpcb, header, response);
    }
    
    // DELETE /api/logs
    if (strcmp(path, "/api/logs") == 0 && strcmp(method, "DELETE") == 0) {
        firewall_clear_logs();
        snprintf(response, sizeof(response), "{\"status\":\"ok\"}");
        return send_response(tpcb, header, response);
    }
    
    // POST /api/blocklist
    if (strcmp(path, "/api/blocklist") == 0 && strcmp(method, "POST") == 0) {
        char ip[16];
        if (web_ui_process_form(body, strlen(body), "ip", sizeof(ip), ip, sizeof(ip))) {
            firewall_block_ip(ip);
            snprintf(response, sizeof(response), "{\"status\":\"ok\",\"ip\":\"%s\"}", ip);
        } else {
            snprintf(response, sizeof(response), "{\"status\":\"error\"}");
        }
        return send_response(tpcb, header, response);
    }
    
    // DELETE /api/blocklist
    if (strcmp(path, "/api/blocklist") == 0 && strcmp(method, "DELETE") == 0) {
        char ip[16];
        if (web_ui_process_form(body, strlen(body), "ip", sizeof(ip), ip, sizeof(ip))) {
            firewall_unblock_ip(ip);
            snprintf(response, sizeof(response), "{\"status\":\"ok\",\"ip\":\"%s\"}", ip);
        } else {
            snprintf(response, sizeof(response), "{\"status\":\"error\"}");
        }
        return send_response(tpcb, header, response);
    }
    
    // GET /api/config
    if (strcmp(path, "/api/config") == 0 && strcmp(method, "GET") == 0) {
        if (g_config) {
            snprintf(response, sizeof(response), "{\"ssid\":\"%s\",\"password\":\"***\"}", g_config->ssid);
        } else {
            snprintf(response, sizeof(response), "{\"ssid\":\"\",\"password\":\"\"}");
        }
        return send_response(tpcb, header, response);
    }
    
    // POST /api/config
    if (strcmp(path, "/api/config") == 0 && strcmp(method, "POST") == 0) {
        if (g_config) {
            char ssid[32], password[64];
            if (web_ui_process_form(body, strlen(body), "ssid", sizeof(ssid), ssid, sizeof(ssid))) {
                strncpy(g_config->ssid, ssid, MAX_SSID_LENGTH - 1);
            }
            if (web_ui_process_form(body, strlen(body), "password", sizeof(password), password, sizeof(password))) {
                strncpy(g_config->password, password, MAX_PASSWORD_LENGTH - 1);
            }
            config_save(g_config);
            snprintf(response, sizeof(response), "{\"status\":\"ok\"}");
        } else {
            snprintf(response, sizeof(response), "{\"status\":\"error\"}");
        }
        return send_response(tpcb, header, response);
    }
    
    // POST /api/reboot
    if (strcmp(path, "/api/reboot") == 0 && strcmp(method, "POST") == 0) {
        snprintf(response, sizeof(response), "{\"status\":\"rebooting\"}");
        send_response(tpcb, header, response);
        sleep_ms(100);
        system_monitor_reset();
        return ERR_OK;
    }
    
    // 404 for unknown endpoints
    snprintf(response, sizeof(response), "{\"error\":\"Not found\"}");
    return send_response(tpcb, "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n", response);
}

// HTTP receive callback
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    
    system_monitor_led_activity();
    
    // Get client IP
    char client_ip[16];
    network_monitor_format_ip(&tpcb->remote_ip, client_ip, sizeof(client_ip));
    
    // Check firewall
    if (firewall_check_ip(client_ip)) {
        printf("Firewall: Blocked connection from %s\n", client_ip);
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_OK;
    }
    
    network_monitor_increment_connections();
    network_monitor_add_bytes_received(p->tot_len);
    
    // Copy request data
    char request[HTTP_BUFFER_SIZE];
    memcpy(request, p->payload, p->len < sizeof(request) - 1 ? p->len : sizeof(request) - 1);
    request[p->len < sizeof(request) - 1 ? p->len : sizeof(request) - 1] = '\0';
    
    pbuf_free(p);
    
    // Parse request
    char method[8], path[128], body[512];
    if (parse_request(request, method, sizeof(method), path, sizeof(path), body, sizeof(body)) != 0) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    
    printf("HTTP: %s %s from %s\n", method, path, client_ip);
    
    // Route request
    if (strncmp(path, "/api/", 5) == 0) {
        handle_api_request(tpcb, method, path, body);
    } else {
        // Serve main dashboard
        size_t html_len;
        const char *html = web_ui_get_page(PAGE_DASHBOARD, &html_len);
        send_response(tpcb, html_header, html);
        
        uint32_t sent = html_len + strlen(html_header);
        network_monitor_add_bytes_sent(sent);
    }
    
    tcp_close(tpcb);
    return ERR_OK;
}

// Connection callback
static err_t http_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (newpcb != NULL) {
        tcp_recv(newpcb, http_recv_callback);
        tcp_nagle_disable(newpcb);
    }
    return ERR_OK;
}

void http_server_init(void) {
    printf("HTTP Server: Initialized\n");
}

void http_server_start(void) {
    if (server_running) {
        printf("HTTP Server: Already running\n");
        return;
    }
    
    server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!server_pcb) {
        printf("HTTP Server: Failed to create PCB\n");
        return;
    }
    
    if (tcp_bind(server_pcb, IP_ADDR_ANY, HTTP_PORT) != ERR_OK) {
        printf("HTTP Server: Failed to bind to port %d\n", HTTP_PORT);
        tcp_close(server_pcb);
        return;
    }
    
    server_pcb = tcp_listen_with_backlog(server_pcb, 4);
    tcp_accept(server_pcb, http_accept_callback);
    
    server_running = true;
    printf("HTTP Server: Started on port %d\n", HTTP_PORT);
}

void http_server_stop(void) {
    if (!server_running) return;
    
    if (server_pcb) {
        tcp_close(server_pcb);
        server_pcb = NULL;
    }
    
    server_running = false;
    printf("HTTP Server: Stopped\n");
}

bool http_server_is_running(void) {
    return server_running;
}

void http_server_poll(void) {
    // TCP polling is handled by lwIP
}

void http_server_set_config(config_t *config) {
    g_config = config;
}
