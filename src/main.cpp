#include "media_json_builder.h"
#include "http_server.h"

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <string>
#include <cctype>

// Возвращает домашний каталог пользователя
// Если переменная HOME не задана, возвращает пустой путь

std::filesystem::path get_home_path() {
    const char* home_path = std::getenv("HOME");

    if (home_path == nullptr) {
        return {};
    }

    return std::filesystem::path(home_path);
}

// Проверка является ли строка положительным целым числом(чтобы отличить интервал от пути к папке)

bool is_positive_number(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    for (char symbol : value) {
        if (!std::isdigit(static_cast<unsigned char>(symbol))) {
            return false;
        }
    }

    return true;
}


std::filesystem::path get_scan_path_from_arguments(int argc,
                                                   char* argv[],
                                                   const std::filesystem::path& home_path) {
    if (argc < 2) {
        return home_path;
    }

    std::string first_argument = argv[1];

    if (is_positive_number(first_argument)) {
        return home_path;
    }

    return std::filesystem::path(first_argument);
}

// Получает интервал обновления из аргументов командной строки
// Если интервал не передан, используется значение по умолчанию(120 секунд)

int get_interval_from_arguments(int argc, char* argv[]) {
    const int default_interval = 120;

    if (argc < 2) {
        return default_interval;
    }

    std::string first_argument = argv[1];

    // Случай: ./kvadra_media_scan 10 [interval_seconds]

    if (is_positive_number(first_argument)) {
        return std::stoi(first_argument);
    }

    // Случай: ./kvadra_media_scan [path] [interval_seconds]

    if (argc >= 3) {
        std::string second_argument = argv[2];

        if (is_positive_number(second_argument)) {
            return std::stoi(second_argument);
        }

        std::cerr << "Warning: invalid interval. Default interval will be used."
                  << std::endl;
    }

    return default_interval;
}

// Периодически пересканирует каталог и обновляет JSON в памяти

void update_json_cache_loop(const std::filesystem::path& scan_path,
                            int interval_seconds,
                            std::string& cached_json,
                            std::mutex& json_mutex) {
    while (true) {
        std::string new_json;

        if (create_media_files_json(scan_path, new_json)) {
            std::lock_guard<std::mutex> lock(json_mutex);
            cached_json = new_json;
        } else {
            std::cerr << "Warning: JSON cache was not updated" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
    }
}

int main(int argc, char* argv[]) {
    const int server_port = 1234;
    const std::string server_host = "localhost";
    const std::string not_found_message = "Not Found";

    // Получаем домашний каталог пользователя

    std::filesystem::path home_path = get_home_path();

    if (home_path.empty()) {
        std::cerr << "Error: HOME environment variable is not set." << std::endl;
        return 1;
    }

    // Получаем каталог для сканирования, а если путь он не передан, используем домашнюю директорию

    std::filesystem::path scan_path = get_scan_path_from_arguments(argc, argv, home_path);

    // Проверка, что выбранный путь существует

    if (!std::filesystem::exists(scan_path)) {
        std::cerr << "Error: scan path does not exist: " << scan_path << std::endl;
        return 1;
    }

    // Проверяем, что выбранный путь является директорией

    if (!std::filesystem::is_directory(scan_path)) {
        std::cerr << "Error: scan path is not a directory: " << scan_path << std::endl;
        return 1;
    }

    int interval_seconds = get_interval_from_arguments(argc, argv);

    std::string cached_json = "{\n  \"audio\": [],\n  \"video\": [],\n  \"images\": []\n}";
    std::mutex json_mutex;

    std::cout << "Scan directory: " << scan_path << std::endl;
    std::cout << "Update interval: " << interval_seconds << " seconds" << std::endl;
    std::cout << "JSON file is created. Result is available via HTTP." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Запускаем отдельный поток, который будет периодически обновлять JSON в памяти

    std::thread updater_thread(update_json_cache_loop,
                               scan_path,
                               interval_seconds,
                               std::ref(cached_json),
                               std::ref(json_mutex));

    updater_thread.detach();

    // Запускаем HTTP-сервер
    // JSON доступен по адресу: http://localhost:1234/media_files

    if (!run_http_server(server_host,
                         server_port,
                         not_found_message,
                         cached_json,
                         json_mutex)) {
        return 1;
    }

    return 0;
}