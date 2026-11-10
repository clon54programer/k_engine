#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "editor/document.h"

namespace k_engine::editor
{

class ProjectIo
{
  public:
    static bool Create(const std::filesystem::path &directory, ProjectBundle &bundle,
                       std::string &error);
    static bool Load(const std::filesystem::path &project_path, ProjectBundle &bundle,
                     std::string &error);
    static bool Save(ProjectBundle &bundle, std::string &error);

    static bool ImportPng(ProjectBundle &bundle, const std::filesystem::path &source_path,
                          std::uint64_t &asset_id, std::string &error);
    static bool CreateLevel(ProjectBundle &bundle, std::uint64_t image_id,
                            std::uint64_t &level_id, std::string &error);
    static bool CreateAnimation(ProjectBundle &bundle, std::uint64_t image_id,
                                std::uint64_t &animation_id, std::string &error);
    static bool AddTileset(ProjectBundle &bundle, Level &level, std::uint64_t image_id,
                           std::uint64_t &tileset_id, std::string &error);
};

} // namespace k_engine::editor
