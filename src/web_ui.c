#include "web_ui.h"
#include "network_monitor.h"
#include "firewall.h"
#include "system_monitor.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include <string.h>
#include <stdio.h>

// HTML Dashboard
static const char dashboard_html[] = 
"<!DOCTYPE html>\n"
"<html lang='en'>\n"
"<head>\n"
"    <meta charset='UTF-8'>\n"
"    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
"    <title>Pico Firewall Monitor</title>\n"
"    <style>\n"
"        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
"        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; color: #fff; }\n"
"        .container { max-width: 1200px; margin: 0 auto; padding: 20px; }\n"
"        header { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 15px; padding: 20px; margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }\n"
"        h1 { font-size: 1.8em; background: linear-gradient(90deg, #00d9ff, #00ff88); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }\n"
"        .status-badge { padding: 8px 16px; border-radius: 20px; font-weight: bold; font-size: 0.9em; }\n"
"        .status-connected { background: linear-gradient(90deg, #00ff88, #00d9ff); color: #000; }\n"
"        .status-disconnected { background: linear-gradient(90deg, #ff4757, #ff6b81); color: #fff; }\n"
"        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; margin-bottom: 20px; }\n"
"        .card { background: rgba(255,255,255,0.05); backdrop-filter: blur(10px); border-radius: 15px; padding: 20px; border: 1px solid rgba(255,255,255,0.1); transition: transform 0.3s, box-shadow 0.3s; }\n"
"        .card:hover { transform: translateY(-5px); box-shadow: 0 10px 30px rgba(0,217,255,0.2); }\n"
"        .card h2 { font-size: 1.2em; margin-bottom: 15px; color: #00d9ff; display: flex; align-items: center; gap: 10px; }\n"
"        .stat { display: flex; justify-content: space-between; padding: 10px 0; border-bottom: 1px solid rgba(255,255,255,0.1); }\n"
"        .stat:last-child { border-bottom: none; }\n"
"        .stat-label { color: #aaa; }\n"
"        .stat-value { font-weight: bold; color: #00ff88; }\n"
"        .nav { display: flex; gap: 10px; flex-wrap: wrap; margin-bottom: 20px; }\n"
"        .nav-btn { padding: 12px 24px; border: none; border-radius: 10px; background: rgba(255,255,255,0.1); color: #fff; cursor: pointer; transition: all 0.3s; font-size: 0.95em; }\n"
"        .nav-btn:hover { background: rgba(0,217,255,0.3); transform: translateY(-2px); }\n"
"        .nav-btn.active { background: linear-gradient(90deg, #00d9ff, #00ff88); color: #000; }\n"
"        .btn { padding: 10px 20px; border: none; border-radius: 8px; cursor: pointer; font-size: 0.9em; transition: all 0.3s; }\n"
"        .btn-primary { background: linear-gradient(90deg, #00d9ff, #00ff88); color: #000; }\n"
"        .btn-danger { background: linear-gradient(90deg, #ff4757, #ff6b81); color: #fff; }\n"
"        .btn:hover { transform: scale(1.05); }\n"
"        table { width: 100%; border-collapse: collapse; margin-top: 15px; }\n"
"        th, td { padding: 12px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }\n"
"        th { color: #00d9ff; font-weight: 600; }\n"
"        tr:hover { background: rgba(255,255,255,0.05); }\n"
"        .input-group { margin-bottom: 15px; }\n"
"        .input-group label { display: block; margin-bottom: 5px; color: #aaa; }\n"
"        .input-group input { width: 100%; padding: 12px; border: 1px solid rgba(255,255,255,0.2); border-radius: 8px; background: rgba(255,255,255,0.05); color: #fff; font-size: 1em; }\n"
"        .input-group input:focus { outline: none; border-color: #00d9ff; }\n"
"        .progress-bar { background: rgba(255,255,255,0.1); border-radius: 10px; height: 20px; overflow: hidden; margin-top: 10px; }\n"
"        .progress-fill { height: 100%; background: linear-gradient(90deg, #00d9ff, #00ff88); transition: width 0.5s; }\n"
"        .log-entry { padding: 10px; border-left: 3px solid #00d9ff; background: rgba(255,255,255,0.03); margin-bottom: 10px; border-radius: 0 8px 8px 0; }\n"
"        .log-entry.blocked { border-left-color: #ff4757; }\n"
"        .log-time { color: #00ff88; font-size: 0.85em; }\n"
"        .page { display: none; }\n"
"        .page.active { display: block; }\n"
"        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }\n"
"        .loading { animation: pulse 1.5s infinite; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class='container'>\n"
"        <header>\n"
"            <h1>🛡️ Pico Firewall Monitor</h1>\n"
"            <span id='connectionStatus' class='status-badge status-disconnected'>Connecting...</span>\n"
"        </header>\n"
"        \n"
"        <nav class='nav'>\n"
"            <button class='nav-btn active' onclick='showPage(\"dashboard\")'>📊 Dashboard</button>\n"
"            <button class='nav-btn' onclick='showPage(\"blocklist\")'>🚫 Blocklist</button>\n"
"            <button class='nav-btn' onclick='showPage(\"logs\")'>📋 Logs</button>\n"
"            <button class='nav-btn' onclick='showPage(\"settings\")'>⚙️ Settings</button>\n"
"        </nav>\n"
"        \n"
"        <div id='dashboard' class='page active'>\n"
"            <div class='grid'>\n"
"                <div class='card'>\n"
"                    <h2>🌐 Network Status</h2>\n"
"                    <div class='stat'><span class='stat-label'>Status</span><span id='wifiStatus' class='stat-value'>-</span></div>\n"
"                    <div class='stat'><span class='stat-label'>IP Address</span><span id='ipAddress' class='stat-value'>-</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Subnet Mask</span><span id='subnetMask' class='stat-value'>-</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Gateway</span><span id='gateway' class='stat-value'>-</span></div>\n"
"                </div>\n"
"                \n"
"                <div class='card'>\n"
"                    <h2>📈 Traffic Statistics</h2>\n"
"                    <div class='stat'><span class='stat-label'>Total Connections</span><span id='totalConnections' class='stat-value'>0</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Active Connections</span><span id='activeConnections' class='stat-value'>0</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Blocked Attempts</span><span id='blockedConnections' class='stat-value'>0</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Bytes Received</span><span id='bytesReceived' class='stat-value'>0</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Bytes Sent</span><span id='bytesSent' class='stat-value'>0</span></div>\n"
"                </div>\n"
"                \n"
"                <div class='card'>\n"
"                    <h2>💻 System Health</h2>\n"
"                    <div class='stat'><span class='stat-label'>Uptime</span><span id='uptime' class='stat-value'>00:00:00</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Memory Usage</span><span id='memoryUsage' class='stat-value'>0%</span></div>\n"
"                    <div class='progress-bar'><div id='memoryProgress' class='progress-fill' style='width: 0%'></div></div>\n"
"                    <div class='stat'><span class='stat-label'>Firewall</span><span id='firewallStatus' class='stat-value'>Enabled</span></div>\n"
"                    <div class='stat'><span class='stat-label'>Blocked IPs</span><span id='blockedIPsCount' class='stat-value'>0</span></div>\n"
"                </div>\n"
"            </div>\n"
"        </div>\n"
"        \n"
"        <div id='blocklist' class='page'>\n"
"            <div class='card'>\n"
"                <h2>🚫 IP Blocklist Management</h2>\n"
"                <div class='input-group'>\n"
"                    <label>Add IP Address:</label>\n"
"                    <div style='display: flex; gap: 10px;'>\n"
"                        <input type='text' id='newBlockedIP' placeholder='192.168.1.100' maxlength='15'>\n"
"                        <button class='btn btn-primary' onclick='addBlockedIP()'>Block IP</button>\n"
"                    </div>\n"
"                </div>\n"
"                <table>\n"
"                    <thead><tr><th>IP Address</th><th>Action</th></tr></thead>\n"
"                    <tbody id='blocklistTable'></tbody>\n"
"                </table>\n"
"            </div>\n"
"        </div>\n"
"        \n"
"        <div id='logs' class='page'>\n"
"            <div class='card'>\n"
"                <h2>📋 Firewall Logs</h2>\n"
"                <button class='btn btn-primary' onclick='refreshLogs()' style='margin-bottom: 15px;'>🔄 Refresh</button>\n"
"                <button class='btn btn-danger' onclick='clearLogs()'>🗑️ Clear Logs</button>\n"
"                <div id='logsContainer' style='margin-top: 15px;'></div>\n"
"            </div>\n"
"        </div>\n"
"        \n"
"        <div id='settings' class='page'>\n"
"            <div class='card'>\n"
"                <h2>⚙️ System Settings</h2>\n"
"                <div class='input-group'>\n"
"                    <label>Wi-Fi SSID:</label>\n"
"                    <input type='text' id='wifiSSID' placeholder='Your WiFi SSID'>\n"
"                </div>\n"
"                <div class='input-group'>\n"
"                    <label>Wi-Fi Password:</label>\n"
"                    <input type='password' id='wifiPassword' placeholder='Your WiFi Password'>\n"
"                </div>\n"
"                <button class='btn btn-primary' onclick='saveSettings()'>💾 Save Settings</button>\n"
"                <button class='btn btn-danger' onclick='rebootDevice()'>🔄 Reboot Device</button>\n"
"            </div>\n"
"        </div>\n"
"    </div>\n"
"    \n"
"    <script>\n"
"        function showPage(pageId) {\n"
"            document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));\n"
"            document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));\n"
"            document.getElementById(pageId).classList.add('active');\n"
"            event.target.classList.add('active');\n"
"            if (pageId === 'blocklist') loadBlocklist();\n"
"            if (pageId === 'logs') refreshLogs();\n"
"            if (pageId === 'settings') loadSettings();\n"
"        }\n"
"        \n"
"        async function fetchData() {\n"
"            try {\n"
"                const response = await fetch('/api/status');\n"
"                const data = await response.json();\n"
"                \n"
"                document.getElementById('wifiStatus').textContent = data.wifi_status;\n"
"                document.getElementById('ipAddress').textContent = data.ip_address;\n"
"                document.getElementById('subnetMask').textContent = data.netmask;\n"
"                document.getElementById('gateway').textContent = data.gateway;\n"
"                document.getElementById('totalConnections').textContent = data.total_connections;\n"
"                document.getElementById('activeConnections').textContent = data.active_connections;\n"
"                document.getElementById('blockedConnections').textContent = data.blocked_connections;\n"
"                document.getElementById('bytesReceived').textContent = formatBytes(data.bytes_received);\n"
"                document.getElementById('bytesSent').textContent = formatBytes(data.bytes_sent);\n"
"                document.getElementById('uptime').textContent = data.uptime;\n"
"                document.getElementById('memoryUsage').textContent = data.memory_usage + '%';\n"
"                document.getElementById('memoryProgress').style.width = data.memory_usage + '%';\n"
"                document.getElementById('blockedIPsCount').textContent = data.blocked_ips_count;\n"
"                \n"
"                const statusEl = document.getElementById('connectionStatus');\n"
"                if (data.wifi_status === 'Connected') {\n"
"                    statusEl.className = 'status-badge status-connected';\n"
"                    statusEl.textContent = 'Connected';\n"
"                } else {\n"
"                    statusEl.className = 'status-badge status-disconnected';\n"
"                    statusEl.textContent = data.wifi_status;\n"
"                }\n"
"            } catch (error) {\n"
"                console.error('Error fetching data:', error);\n"
"            }\n"
"        }\n"
"        \n"
"        function formatBytes(bytes) {\n"
"            if (bytes === 0) return '0 B';\n"
"            const k = 1024;\n"
"            const sizes = ['B', 'KB', 'MB', 'GB'];\n"
"            const i = Math.floor(Math.log(bytes) / Math.log(k));\n"
"            return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];\n"
"        }\n"
"        \n"
"        async function loadBlocklist() {\n"
"            try {\n"
"                const response = await fetch('/api/blocklist');\n"
"                const data = await response.json();\n"
"                const tbody = document.getElementById('blocklistTable');\n"
"                tbody.innerHTML = '';\n"
"                data.ips.forEach(ip => {\n"
"                    const row = tbody.insertRow();\n"
"                    row.innerHTML = '<td>' + ip + '</td><td><button class=\"btn btn-danger\" onclick=\"unblockIP(\\'' + ip + '\\')\">Unblock</button></td>';\n"
"                });\n"
"            } catch (error) {\n"
"                console.error('Error loading blocklist:', error);\n"
"            }\n"
"        }\n"
"        \n"
"        async function addBlockedIP() {\n"
"            const ip = document.getElementById('newBlockedIP').value;\n"
"            if (!ip) return alert('Please enter an IP address');\n"
"            await fetch('/api/blocklist', { method: 'POST', body: 'ip=' + ip, headers: { 'Content-Type': 'application/x-www-form-urlencoded' } });\n"
"            document.getElementById('newBlockedIP').value = '';\n"
"            loadBlocklist();\n"
"            fetchData();\n"
"        }\n"
"        \n"
"        async function unblockIP(ip) {\n"
"            await fetch('/api/blocklist', { method: 'DELETE', body: 'ip=' + ip, headers: { 'Content-Type': 'application/x-www-form-urlencoded' } });\n"
"            loadBlocklist();\n"
"            fetchData();\n"
"        }\n"
"        \n"
"        async function refreshLogs() {\n"
"            try {\n"
"                const response = await fetch('/api/logs');\n"
"                const data = await response.json();\n"
"                const container = document.getElementById('logsContainer');\n"
"                container.innerHTML = '';\n"
"                data.logs.forEach(log => {\n"
"                    const div = document.createElement('div');\n"
"                    div.className = 'log-entry' + (log.blocked ? ' blocked' : '');\n"
"                    div.innerHTML = '<span class=\"log-time\">[Uptime: ' + log.uptime + 's]</span> ' + log.ip + ' - ' + log.action;\n"
"                    container.appendChild(div);\n"
"                });\n"
"            } catch (error) {\n"
"                console.error('Error loading logs:', error);\n"
"            }\n"
"        }\n"
"        \n"
"        async function clearLogs() {\n"
"            await fetch('/api/logs', { method: 'DELETE' });\n"
"            refreshLogs();\n"
"        }\n"
"        \n"
"        async function loadSettings() {\n"
"            try {\n"
"                const response = await fetch('/api/config');\n"
"                const data = await response.json();\n"
"                document.getElementById('wifiSSID').value = data.ssid || '';\n"
"                document.getElementById('wifiPassword').value = data.password || '';\n"
"            } catch (error) {\n"
"                console.error('Error loading settings:', error);\n"
"            }\n"
"        }\n"
"        \n"
"        async function saveSettings() {\n"
"            const ssid = document.getElementById('wifiSSID').value;\n"
"            const password = document.getElementById('wifiPassword').value;\n"
"            await fetch('/api/config', { method: 'POST', body: 'ssid=' + ssid + '&password=' + password, headers: { 'Content-Type': 'application/x-www-form-urlencoded' } });\n"
"            alert('Settings saved! Reboot required to apply Wi-Fi changes.');\n"
"        }\n"
"        \n"
"        async function rebootDevice() {\n"
"            if (confirm('Are you sure you want to reboot the device?')) {\n"
"                await fetch('/api/reboot', { method: 'POST' });\n"
"                alert('Device is rebooting...');\n"
"            }\n"
"        }\n"
"        \n"
"        // Auto-refresh every 3 seconds\n"
"        setInterval(fetchData, 3000);\n"
"        fetchData();\n"
"    </script>\n"
"</body>\n"
"</html>\n";

const char* web_ui_get_page(page_type_t page, size_t *length) {
    if (length) *length = sizeof(dashboard_html) - 1;
    return dashboard_html;
}

const char* web_ui_get_content_type(page_type_t page) {
    return "text/html";
}

bool web_ui_process_form(const char *form_data, size_t length, char *key, size_t key_size, 
                         char *value, size_t value_size) {
    if (!form_data || length == 0) return false;
    
    const char *key_pos = strstr(form_data, key);
    if (!key_pos) return false;
    
    const char *value_start = strchr(key_pos, '=');
    if (!value_start) return false;
    value_start++;
    
    const char *value_end = strchr(value_start, '&');
    size_t value_len = value_end ? (value_end - value_start) : strlen(value_start);
    
    if (value_len >= value_size) value_len = value_size - 1;
    
    strncpy(value, value_start, value_len);
    value[value_len] = '\0';
    
    return true;
}

void web_ui_generate_status_json(char *buffer, size_t buffer_size) {
    network_stats_t stats;
    network_monitor_update_stats(&stats);
    
    system_health_t health;
    system_monitor_get_health(&health);
    
    char uptime_str[32];
    system_monitor_get_uptime_string(uptime_str, sizeof(uptime_str));
    
    const char *wifi_status = network_monitor_get_status_string();
    
    uint8_t *ip = (uint8_t *)&cyw43_state.netif[0].ip_addr.addr;
    uint8_t *nm = (uint8_t *)&cyw43_state.netif[0].netmask.addr;
    uint8_t *gw = (uint8_t *)&cyw43_state.netif[0].gw.addr;
    
    snprintf(buffer, buffer_size,
        "{"
        "\"wifi_status\":\"%s\","
        "\"ip_address\":\"%d.%d.%d.%d\","
        "\"netmask\":\"%d.%d.%d.%d\","
        "\"gateway\":\"%d.%d.%d.%d\","
        "\"total_connections\":%lu,"
        "\"active_connections\":%lu,"
        "\"blocked_connections\":%lu,"
        "\"bytes_received\":%lu,"
        "\"bytes_sent\":%lu,"
        "\"uptime\":\"%s\","
        "\"memory_usage\":%.1f,"
        "\"blocked_ips_count\":%d,"
        "\"firewall_enabled\":%s"
        "}",
        wifi_status,
        ip[0], ip[1], ip[2], ip[3],
        nm[0], nm[1], nm[2], nm[3],
        gw[0], gw[1], gw[2], gw[3],
        stats.total_connections,
        stats.active_connections,
        stats.blocked_connections,
        stats.bytes_received,
        stats.bytes_sent,
        uptime_str,
        system_monitor_get_memory_usage_percent(),
        firewall_get_blocked_ips_count(),
        firewall_is_enabled() ? "true" : "false"
    );
}

void web_ui_generate_config_json(char *buffer, size_t buffer_size) {
    // Config is accessed through firewall module
    snprintf(buffer, buffer_size, "{\"ssid\":\"\",\"password\":\"\"}");
}

void web_ui_generate_blocklist_json(char *buffer, size_t buffer_size) {
    buffer[0] = '{';
    buffer[1] = '"';
    buffer[2] = 'i';
    buffer[3] = 'p';
    buffer[4] = 's';
    buffer[5] = '"';
    buffer[6] = ':';
    buffer[7] = '[';
    
    size_t pos = 8;
    int count = firewall_get_blocked_ips_count();
    
    for (int i = 0; i < count && pos < buffer_size - 10; i++) {
        const char *ip = firewall_get_blocked_ip(i);
        if (ip) {
            if (i > 0) buffer[pos++] = ',';
            int written = snprintf(buffer + pos, buffer_size - pos, "\"%s\"", ip);
            pos += written;
        }
    }
    
    buffer[pos++] = ']';
    buffer[pos++] = '}';
    buffer[pos] = '\0';
}

void web_ui_generate_logs_json(char *buffer, size_t buffer_size) {
    firewall_log_t logs[MAX_FIREWALL_LOGS];
    int count = firewall_get_logs(logs, MAX_FIREWALL_LOGS);
    
    buffer[0] = '{';
    buffer[1] = '"';
    buffer[2] = 'l';
    buffer[3] = 'o';
    buffer[4] = 'g';
    buffer[5] = 's';
    buffer[6] = '"';
    buffer[7] = ':';
    buffer[8] = '[';
    
    size_t pos = 9;
    for (int i = 0; i < count && pos < buffer_size - 20; i++) {
        if (logs[i].timestamp == 0) continue;
        
        if (i > 0 && buffer[pos-1] != '[') buffer[pos++] = ',';
        
        int written = snprintf(buffer + pos, buffer_size - pos,
            "{\"uptime\":%lu,\"ip\":\"%s\",\"action\":\"%s\",\"blocked\":%s}",
            logs[i].timestamp, logs[i].ip_address, logs[i].action,
            logs[i].blocked ? "true" : "false"
        );
        pos += written;
    }
    
    buffer[pos++] = ']';
    buffer[pos++] = '}';
    buffer[pos] = '\0';
}
