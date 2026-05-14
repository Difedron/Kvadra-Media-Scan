#include "http_server.h"

#include <iostream>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Отправляет HTTP-ответ клиенту.

void send_response(int client_socket,
                   const std::string& status,
                   const std::string& content_type,
                   const std::string& body) {
    std::string response =
            "HTTP/1.1 " + status + "\r\n" +
            "Content-Type: " + content_type + "\r\n" +
            "Content-Length: " + std::to_string(body.size()) + "\r\n" +
            "Connection: close\r\n" +
            "\r\n" +
            body;

    send(client_socket, response.c_str(), response.size(), 0);
}

// Обрабатывает один входящий HTTP-запрос.

void handle_client(int client_socket,
                   const std::string& not_found_message,
                   std::string& cached_json,
                   std::mutex& json_mutex) {
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    std::string request(buffer);

    // Поддерживаем только GET /media_files.

    if (request.rfind("GET /media_files ", 0) == 0 ||
        request.rfind("GET /media_files HTTP/", 0) == 0) {
        std::string json_copy;

        {
            std::lock_guard<std::mutex> lock(json_mutex);
            json_copy = cached_json;
        }

        send_response(client_socket,
                      "200 OK",
                      "application/json",
                      json_copy);
    } else {
        send_response(client_socket,
                      "404 Not Found",
                      "text/plain",
                      not_found_message);
    }

    close(client_socket);
}

// Запускает простой HTTP-сервер на указанном порту.

bool run_http_server(const std::string& server_host,
                     int port,
                     const std::string& not_found_message,
                     std::string& cached_json,
                     std::mutex& json_mutex) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return false;
    }

    int option = 1;
    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(port);

    if (bind(server_socket,
             reinterpret_cast<sockaddr*>(&server_address),
             sizeof(server_address)) < 0) {
        std::cerr << "Error: cannot bind server to port " << port << std::endl;
        close(server_socket);
        return false;
    }

    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error: cannot listen on port " << port << std::endl;
        close(server_socket);
        return false;
    }

    std::cout << "HTTP server started: http://" << server_host << ":"
              << port << "/media_files" << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);

        if (client_socket < 0) {
            std::cerr << "Warning: cannot accept client connection" << std::endl;
            continue;
        }

        handle_client(client_socket,
                      not_found_message,
                      cached_json,
                      json_mutex);
    }

    close(server_socket);
    return true;
}