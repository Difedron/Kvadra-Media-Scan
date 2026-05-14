//
// Created by Diana Kholukhina on 13.05.2026.
//

#ifndef KVADRAMEDIASCAN_MEDIA_JSON_BUILDER_H
#define KVADRAMEDIASCAN_MEDIA_JSON_BUILDER_H

#include <filesystem>
#include <string>

// Сканирует указанный каталог и формирует JSON со списком найденных медиафайлов

bool create_media_files_json(const std::filesystem::path& scan_path,
                             std::string& json_result);

#endif // KVADRAMEDIASCAN_MEDIA_JSON_BUILDER_H
