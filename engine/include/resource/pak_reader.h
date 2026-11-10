#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace k_engine::resource
{

enum class PakResourceType : std::uint8_t
{
    LEVEL = 1,
    ANIMATION = 2,
    PNG = 3,
};

struct PakEntry
{
    std::uint64_t id = 0;
    PakResourceType type = PakResourceType::LEVEL;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::uint32_t crc32 = 0;
};

// Lector del contenedor .pak v1. Abre el archivo, lee la cabecera y la TOC, y
// entrega cada payload bajo demanda sin cargar el paquete completo en memoria.
class PakReader
{
  public:
    PakReader() = default;
    ~PakReader() = default;

    PakReader(const PakReader &) = delete;
    PakReader &operator=(const PakReader &) = delete;
    PakReader(PakReader &&) = delete;
    PakReader &operator=(PakReader &&) = delete;

    bool Open(const std::string &path);
    void Close() noexcept;

    const std::vector<PakEntry> &Entries() const noexcept;
    bool Read(std::uint64_t resource_id, PakResourceType expected_type,
              std::vector<std::uint8_t> &out) const;

    const std::string &LastError() const noexcept;
    const std::string &Path() const noexcept;

  private:
    bool fail(std::string message) const;
    bool read_file_size();
    bool read_header_and_toc();
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool parse_and_validate_entries(const std::vector<std::uint8_t> &toc,
                                    std::uint32_t count, std::uint64_t data_start);
    const PakEntry *find_entry(std::uint64_t resource_id) const noexcept;

    mutable std::ifstream _file;
    std::string _path;
    mutable std::string _last_error;
    std::uint64_t _file_size = 0;
    std::uint64_t _data_start = 0;
    std::vector<PakEntry> _entries;
};

} // namespace k_engine::resource
