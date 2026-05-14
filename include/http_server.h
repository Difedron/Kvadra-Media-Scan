//
// Created by Diana Kholukhina on 14.05.2026.
//

#ifndef KVADRAMEDIASCAN_HTTP_SERVER_H
#define KVADRAMEDIASCAN_HTTP_SERVER_H

#include <mutex>
#include <string>

// Запускает HTTP-сервер.
// JSON доступен по адресу: http://localhost:1234/media_files
bool run_http_server(const std::string& server_host,
                     int port,
                     const std::string& not_found_message,
                     std::string& cached_json,
                     std::mutex& json_mutex);

#endif // KVADRAMEDIASCAN_HTTP_SERVER_H