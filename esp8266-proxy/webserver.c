/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Johan Kanflo (github.com/kanflo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <esp8266.h>
#include <espressif/esp_common.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <lwip/api.h>
#include <lwip/tcp.h>
#include "webserver.h"
#include "protocol.h"
#include "uframe.h"

/** User friendly FreeRTOS delay macro */
#define delay_ms(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

/** UART communication callback */
static uart_comm_func_t g_uart_comm = NULL;

/** Maximum request size */
#define MAX_REQUEST_SIZE 512

/** Maximum response size */
#define MAX_RESPONSE_SIZE 2048

/** Embedded web page HTML */
static const char index_html[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<title>OpenDPS</title>"
"<style>"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;min-height:100vh;padding:20px}"
".container{max-width:400px;margin:0 auto}"
"h1{text-align:center;color:#0f0;margin-bottom:20px;font-size:24px}"
".card{background:#16213e;border-radius:10px;padding:20px;margin-bottom:15px}"
".status{display:grid;grid-template-columns:1fr 1fr;gap:10px}"
".stat{text-align:center;padding:15px;background:#0f3460;border-radius:8px}"
".stat-label{font-size:12px;color:#888;margin-bottom:5px}"
".stat-value{font-size:24px;font-weight:bold}"
".stat-value.voltage{color:#0f0}"
".stat-value.current{color:#ff0}"
".stat-value.power{color:#f0f}"
".stat-value.input{color:#0ff}"
".controls{margin-top:15px}"
".control-group{margin-bottom:15px}"
".control-group label{display:block;margin-bottom:5px;color:#888;font-size:14px}"
".control-row{display:flex;gap:10px}"
".control-row input{flex:1;padding:10px;border:none;border-radius:5px;background:#0f3460;color:#fff;font-size:16px}"
".control-row button{padding:10px 20px;border:none;border-radius:5px;cursor:pointer;font-size:14px;font-weight:bold}"
".btn-set{background:#0f0;color:#000}"
".btn-on{background:#0f0;color:#000;width:100%;padding:15px;font-size:18px}"
".btn-off{background:#f00;color:#fff;width:100%;padding:15px;font-size:18px}"
".btn-set:hover{background:#0c0}"
".btn-on:hover{background:#0c0}"
".btn-off:hover{background:#c00}"
".output-status{text-align:center;padding:10px;border-radius:5px;margin-bottom:10px;font-weight:bold}"
".output-on{background:#0f03;border:2px solid #0f0;color:#0f0}"
".output-off{background:#f003;border:2px solid #f00;color:#f00}"
".error{color:#f00;text-align:center;padding:10px}"
"</style>"
"</head>"
"<body>"
"<div class=\"container\">"
"<h1>OpenDPS Control</h1>"
"<div class=\"card\">"
"<div class=\"status\">"
"<div class=\"stat\"><div class=\"stat-label\">Output Voltage</div><div class=\"stat-value voltage\" id=\"vout\">--</div></div>"
"<div class=\"stat\"><div class=\"stat-label\">Output Current</div><div class=\"stat-value current\" id=\"iout\">--</div></div>"
"<div class=\"stat\"><div class=\"stat-label\">Input Voltage</div><div class=\"stat-value input\" id=\"vin\">--</div></div>"
"<div class=\"stat\"><div class=\"stat-label\">Power</div><div class=\"stat-value power\" id=\"pout\">--</div></div>"
"</div>"
"</div>"
"<div class=\"card\">"
"<div id=\"output-status\" class=\"output-status output-off\">OUTPUT OFF</div>"
"<button id=\"output-btn\" class=\"btn-on\" onclick=\"toggleOutput()\">ENABLE OUTPUT</button>"
"</div>"
"<div class=\"card controls\">"
"<div class=\"control-group\">"
"<label>Voltage Setpoint (V)</label>"
"<div class=\"control-row\">"
"<input type=\"number\" id=\"voltage\" step=\"0.01\" min=\"0\" max=\"50\" placeholder=\"5.00\">"
"<button class=\"btn-set\" onclick=\"setVoltage()\">SET</button>"
"</div>"
"</div>"
"<div class=\"control-group\">"
"<label>Current Limit (A)</label>"
"<div class=\"control-row\">"
"<input type=\"number\" id=\"current\" step=\"0.001\" min=\"0\" max=\"5\" placeholder=\"1.000\">"
"<button class=\"btn-set\" onclick=\"setCurrent()\">SET</button>"
"</div>"
"</div>"
"</div>"
"<div id=\"error\" class=\"error\"></div>"
"</div>"
"<script>"
"var outputEnabled=false;"
"function updateStatus(){"
"fetch('/api/status').then(r=>r.json()).then(d=>{"
"document.getElementById('vout').textContent=d.v_out.toFixed(2)+'V';"
"document.getElementById('iout').textContent=d.i_out.toFixed(3)+'A';"
"document.getElementById('vin').textContent=d.v_in.toFixed(2)+'V';"
"document.getElementById('pout').textContent=(d.v_out*d.i_out).toFixed(2)+'W';"
"outputEnabled=d.output_enabled;"
"var btn=document.getElementById('output-btn');"
"var st=document.getElementById('output-status');"
"if(outputEnabled){"
"btn.className='btn-off';btn.textContent='DISABLE OUTPUT';"
"st.className='output-status output-on';st.textContent='OUTPUT ON';"
"}else{"
"btn.className='btn-on';btn.textContent='ENABLE OUTPUT';"
"st.className='output-status output-off';st.textContent='OUTPUT OFF';"
"}"
"document.getElementById('error').textContent='';"
"}).catch(e=>{document.getElementById('error').textContent='Connection error';});"
"}"
"function setVoltage(){"
"var v=parseFloat(document.getElementById('voltage').value);"
"if(isNaN(v)){alert('Invalid voltage');return;}"
"fetch('/api/voltage',{method:'POST',body:v.toFixed(2)}).then(r=>r.json()).then(d=>{"
"if(!d.success)document.getElementById('error').textContent='Failed to set voltage';"
"else updateStatus();"
"}).catch(e=>{document.getElementById('error').textContent='Connection error';});"
"}"
"function setCurrent(){"
"var i=parseFloat(document.getElementById('current').value);"
"if(isNaN(i)){alert('Invalid current');return;}"
"fetch('/api/current',{method:'POST',body:i.toFixed(3)}).then(r=>r.json()).then(d=>{"
"if(!d.success)document.getElementById('error').textContent='Failed to set current';"
"else updateStatus();"
"}).catch(e=>{document.getElementById('error').textContent='Connection error';});"
"}"
"function toggleOutput(){"
"fetch('/api/output',{method:'POST',body:outputEnabled?'0':'1'}).then(r=>r.json()).then(d=>{"
"if(!d.success)document.getElementById('error').textContent='Failed to toggle output';"
"else updateStatus();"
"}).catch(e=>{document.getElementById('error').textContent='Connection error';});"
"}"
"updateStatus();"
"setInterval(updateStatus,1000);"
"</script>"
"</body>"
"</html>";

/** HTTP response headers */
static const char http_html_header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";
static const char http_json_header[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
static const char http_404[] = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\nNot Found";
static const char http_options[] = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\nConnection: close\r\n\r\n";

/**
 * @brief Create a query frame to get DPS status
 */
static void create_query_frame(frame_t *frame)
{
    set_frame_header(frame);
    pack8(frame, cmd_query);
    end_frame(frame);
}

/**
 * @brief Create a set parameters frame
 * @param frame Output frame
 * @param param_name Parameter name (e.g., "voltage" or "current")
 * @param value Parameter value as string (e.g., "5.00")
 */
static void create_set_param_frame(frame_t *frame, const char *param_name, const char *value)
{
    set_frame_header(frame);
    pack8(frame, cmd_set_parameters);
    pack_cstr(frame, param_name);
    pack_cstr(frame, value);
    end_frame(frame);
}

/**
 * @brief Create an enable output frame
 * @param frame Output frame
 * @param enable 1 to enable, 0 to disable
 */
static void create_enable_output_frame(frame_t *frame, uint8_t enable)
{
    set_frame_header(frame);
    pack8(frame, cmd_enable_output);
    pack8(frame, enable);
    end_frame(frame);
}

/**
 * @brief Parse query response and format as JSON
 * @param frame Response frame from DPS
 * @param json_buf Output buffer for JSON
 * @param buf_size Size of output buffer
 * @return true on success
 */
static bool parse_query_response(frame_t *frame, char *json_buf, size_t buf_size)
{
    uint8_t cmd, status, output_enabled, temp_shutdown;
    uint16_t v_in, v_out, i_out;
    int16_t temp1, temp2;

    start_frame_unpacking(frame);
    UNPACK8(frame, &cmd);
    UNPACK8(frame, &status);

    if (cmd != (cmd_response | cmd_query) || !status) {
        snprintf(json_buf, buf_size, "{\"error\":\"invalid response\"}");
        return false;
    }

    UNPACK16(frame, &v_in);
    UNPACK16(frame, &v_out);
    UNPACK16(frame, &i_out);
    UNPACK8(frame, &output_enabled);
    UNPACK16(frame, &temp1);
    UNPACK16(frame, &temp2);
    UNPACK8(frame, &temp_shutdown);
    // Skip cur_func string and parameters for basic JSON response

    // Values are in mV and mA, convert to V and A
    snprintf(json_buf, buf_size,
        "{\"v_in\":%.2f,\"v_out\":%.2f,\"i_out\":%.3f,\"output_enabled\":%s}",
        v_in / 1000.0f,
        v_out / 1000.0f,
        i_out / 1000.0f,
        output_enabled ? "true" : "false");

    return true;
}

/**
 * @brief Parse simple response (for set commands)
 * @param frame Response frame from DPS
 * @return true if command succeeded
 */
static bool parse_simple_response(frame_t *frame)
{
    uint8_t cmd, status;
    start_frame_unpacking(frame);
    UNPACK8(frame, &cmd);
    UNPACK8(frame, &status);
    return status != 0;
}

/**
 * @brief Find start of HTTP body
 */
static char* find_body(char *request)
{
    char *body = strstr(request, "\r\n\r\n");
    if (body) {
        return body + 4;
    }
    return NULL;
}

/**
 * @brief Handle incoming HTTP request
 */
static void handle_request(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;
    char response[MAX_RESPONSE_SIZE];

    err = netconn_recv(conn, &inbuf);
    if (err != ERR_OK) {
        return;
    }

    netbuf_data(inbuf, (void**)&buf, &buflen);

    if (buflen > 0) {
        // Null terminate for string operations
        if (buflen < MAX_REQUEST_SIZE - 1) {
            buf[buflen] = '\0';
        } else {
            buf[MAX_REQUEST_SIZE - 1] = '\0';
        }

        // Handle OPTIONS (CORS preflight)
        if (strncmp(buf, "OPTIONS", 7) == 0) {
            netconn_write(conn, http_options, strlen(http_options), NETCONN_NOCOPY);
        }
        // GET /
        else if (strncmp(buf, "GET / ", 6) == 0 || strncmp(buf, "GET /index", 10) == 0) {
            netconn_write(conn, http_html_header, strlen(http_html_header), NETCONN_NOCOPY);
            netconn_write(conn, index_html, strlen(index_html), NETCONN_NOCOPY);
        }
        // GET /api/status
        else if (strncmp(buf, "GET /api/status", 15) == 0) {
            frame_t frame;
            create_query_frame(&frame);

            if (g_uart_comm && g_uart_comm(&frame)) {
                char json[256];
                parse_query_response(&frame, json, sizeof(json));
                snprintf(response, sizeof(response), "%s%s", http_json_header, json);
            } else {
                snprintf(response, sizeof(response), "%s{\"error\":\"communication timeout\"}", http_json_header);
            }
            netconn_write(conn, response, strlen(response), NETCONN_COPY);
        }
        // POST /api/voltage
        else if (strncmp(buf, "POST /api/voltage", 17) == 0) {
            char *body = find_body(buf);
            if (body && g_uart_comm) {
                frame_t frame;
                create_set_param_frame(&frame, "voltage", body);

                bool success = g_uart_comm(&frame) && parse_simple_response(&frame);
                snprintf(response, sizeof(response), "%s{\"success\":%s}",
                    http_json_header, success ? "true" : "false");
            } else {
                snprintf(response, sizeof(response), "%s{\"success\":false,\"error\":\"no body\"}", http_json_header);
            }
            netconn_write(conn, response, strlen(response), NETCONN_COPY);
        }
        // POST /api/current
        else if (strncmp(buf, "POST /api/current", 17) == 0) {
            char *body = find_body(buf);
            if (body && g_uart_comm) {
                frame_t frame;
                create_set_param_frame(&frame, "current", body);

                bool success = g_uart_comm(&frame) && parse_simple_response(&frame);
                snprintf(response, sizeof(response), "%s{\"success\":%s}",
                    http_json_header, success ? "true" : "false");
            } else {
                snprintf(response, sizeof(response), "%s{\"success\":false,\"error\":\"no body\"}", http_json_header);
            }
            netconn_write(conn, response, strlen(response), NETCONN_COPY);
        }
        // POST /api/output
        else if (strncmp(buf, "POST /api/output", 16) == 0) {
            char *body = find_body(buf);
            if (body && g_uart_comm) {
                frame_t frame;
                uint8_t enable = (body[0] == '1') ? 1 : 0;
                create_enable_output_frame(&frame, enable);

                bool success = g_uart_comm(&frame) && parse_simple_response(&frame);
                snprintf(response, sizeof(response), "%s{\"success\":%s}",
                    http_json_header, success ? "true" : "false");
            } else {
                snprintf(response, sizeof(response), "%s{\"success\":false,\"error\":\"no body\"}", http_json_header);
            }
            netconn_write(conn, response, strlen(response), NETCONN_COPY);
        }
        // 404 for everything else
        else {
            netconn_write(conn, http_404, strlen(http_404), NETCONN_NOCOPY);
        }
    }

    netconn_close(conn);
    netbuf_delete(inbuf);
}

void webserver_init(uart_comm_func_t comm_func)
{
    g_uart_comm = comm_func;
}

void webserver_task(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    (void)pvParameters;

    conn = netconn_new(NETCONN_TCP);
    if (conn == NULL) {
        printf("Failed to create netconn\n");
        vTaskDelete(NULL);
        return;
    }

    err = netconn_bind(conn, NULL, HTTP_PORT);
    if (err != ERR_OK) {
        printf("Failed to bind to port %d\n", HTTP_PORT);
        netconn_delete(conn);
        vTaskDelete(NULL);
        return;
    }

    netconn_listen(conn);
    printf("Web server listening on port %d\n", HTTP_PORT);

    while (1) {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            handle_request(newconn);
            netconn_delete(newconn);
        }
    }
}
