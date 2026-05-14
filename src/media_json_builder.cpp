#include "media_json_builder.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <system_error>

// Получает расширение файла и приводит его к нижнему регистру
// Это нужно, чтобы .MP3, .Mp3 и .mp3 считались одинаковыми расширениями

std::string get_lowercase_extension(const std::filesystem::path& file_path) {
    std::string extension = file_path.extension().string();

    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char symbol) {
                       return std::tolower(symbol);
                   });

    return extension;
}

// Проверка относится ли файл к аудиофайлу

bool is_audio_file(const std::string& extension) {
    return extension == ".mp3" ||
           extension == ".wav" ||
           extension == ".flac" ||
           extension == ".aac" ||
           extension == ".ogg";
}

// Проверка относится ли файл к видеофайлу

bool is_video_file(const std::string& extension) {
    return extension == ".mp4" ||
           extension == ".mpg" ||
           extension == ".mpeg" ||
           extension == ".avi" ||
           extension == ".mkv" ||
           extension == ".mov";
}

// Проверка относится ли файл к изображению

bool is_image_file(const std::string& extension) {
    return extension == ".jpg" ||
           extension == ".jpeg" ||
           extension == ".png" ||
           extension == ".bmp" ||
           extension == ".gif" ||
           extension == ".webp";
}

// Преобразуем список имён в JSON

std::string make_json_array(const std::vector<std::string>& files) {
    std::ostringstream json;

    json << "[";

    for (std::size_t i = 0; i < files.size(); ++i) {
        json << "\"" << files[i] << "\"";

        if (i + 1 < files.size()) {
            json << ", ";
        }
    }

    json << "]";

    return json.str();
}


std::string make_json(const std::vector<std::string>& audio_files,
                      const std::vector<std::string>& video_files,
                      const std::vector<std::string>& image_files) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"audio\": " << make_json_array(audio_files) << ",\n";
    json << "  \"video\": " << make_json_array(video_files) << ",\n";
    json << "  \"images\": " << make_json_array(image_files) << "\n";
    json << "}";

    return json.str();
}

// Определяет категорию файла по расширению и добавляет его имя

void add_file_to_category(const std::filesystem::path& file_path,
                          std::vector<std::string>& audio_files,
                          std::vector<std::string>& video_files,
                          std::vector<std::string>& image_files) {
    std::string extension = get_lowercase_extension(file_path);
    std::string file_name = file_path.filename().string();

    if (is_audio_file(extension)) {
        audio_files.push_back(file_name);
    } else if (is_video_file(extension)) {
        video_files.push_back(file_name);
    } else if (is_image_file(extension)) {
        image_files.push_back(file_name);
    }
}


// Недоступные директории пропускаются, чтобы избежать ошибки

bool scan_directory(const std::filesystem::path& scan_path,
                    std::vector<std::string>& audio_files,
                    std::vector<std::string>& video_files,
                    std::vector<std::string>& image_files) {
    std::filesystem::directory_options options =
            std::filesystem::directory_options::skip_permission_denied;

    std::error_code error_code;

    std::filesystem::recursive_directory_iterator iterator(scan_path, options, error_code);
    std::filesystem::recursive_directory_iterator end;

    if (error_code) {
        std::cerr << "Error: cannot open scan directory: "
                  << error_code.message() << std::endl;
        return false;
    }

    while (iterator != end) {
        std::filesystem::directory_entry entry = *iterator;

        if (entry.is_regular_file(error_code)) {
            add_file_to_category(entry.path(), audio_files, video_files, image_files);
        }

        error_code.clear();
        iterator.increment(error_code);

        if (error_code) {
            error_code.clear();
        }
    }

    return true;
}


// Сканирует каталог и возвращает JSON с найденными медиафайлами

bool create_media_files_json(const std::filesystem::path& scan_path,
                             std::string& json_result) {
    std::vector<std::string> audio_files;
    std::vector<std::string> video_files;
    std::vector<std::string> image_files;

    if (!scan_directory(scan_path, audio_files, video_files, image_files)) {
        return false;
    }

    json_result = make_json(audio_files, video_files, image_files);

    std::cout << "Media JSON was rebuilt" << std::endl;
    std::cout << "Audio files: " << audio_files.size() << std::endl;
    std::cout << "Video files: " << video_files.size() << std::endl;
    std::cout << "Image files: " << image_files.size() << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    return true;
}