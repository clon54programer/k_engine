#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include "editor/document.h"

namespace k_engine::editor
{

inline constexpr std::uint32_t MAX_IMAGE_DIMENSION = 4096;
inline constexpr std::uint64_t MAX_DECODED_IMAGE_BYTES = 64ULL * 1024ULL * 1024ULL;
inline constexpr std::uint32_t MAX_GRID_DIMENSION = 4096;
inline constexpr std::uint64_t MAX_GRID_CELLS = 1'048'576ULL;
inline constexpr std::uint32_t MAX_TILESETS = 64;
inline constexpr std::uint32_t MAX_LAYERS = 16;
inline constexpr std::uint32_t MAX_ENTITIES = 65'536;
inline constexpr std::uint32_t MAX_FRAMES = 4096;
inline constexpr std::uint32_t MIN_FPS = 1;
inline constexpr std::uint32_t MAX_FPS = 240;
inline constexpr std::uint32_t MAX_FRAME_DURATION_MS = 60'000;
inline constexpr std::size_t MAX_STRING_BYTES = 1024 * 1024;
inline constexpr std::size_t MAX_PATH_BYTES = 4096;
inline constexpr std::uint64_t MAX_PROJECT_FILE_BYTES = 4ULL * 1024ULL * 1024ULL;
inline constexpr std::uint64_t MAX_DOCUMENT_FILE_BYTES = 64ULL * 1024ULL * 1024ULL;

bool ParseId(std::string_view text, std::uint64_t &value, std::string &error);
std::string FormatId(std::uint64_t value);
bool IsSafeRelativePath(std::string_view path, std::string &error);
bool ReadPngInfo(const std::filesystem::path &path, std::uint32_t &width,
                 std::uint32_t &height, std::string &error);

bool ValidateProject(const ProjectBundle &bundle, std::string &error);
bool ValidateLevel(const Level &level, std::string &error);
bool ValidateAnimation(const Animation &animation, std::string &error);
bool ValidateProjectFiles(const ProjectBundle &bundle, std::string &error);
bool ValidateAll(const ProjectBundle &bundle, std::string &error);

} // namespace k_engine::editor
