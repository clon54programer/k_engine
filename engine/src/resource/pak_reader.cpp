#include "resource/pak_reader.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "resource/crc32.h"

namespace k_engine::resource
{

namespace
{

constexpr std::size_t HEADER_SIZE = 16;
constexpr std::size_t TOC_ENTRY_SIZE = 40;
constexpr std::size_t HEADER_VERSION_MAJOR_OFFSET = 4;
constexpr std::size_t HEADER_VERSION_MINOR_OFFSET = 6;
constexpr std::size_t HEADER_COUNT_OFFSET = 8;
constexpr std::size_t HEADER_TOC_CRC_OFFSET = 12;
constexpr std::size_t ENTRY_ID_OFFSET = 0;
constexpr std::size_t ENTRY_TYPE_OFFSET = 8;
constexpr std::size_t ENTRY_FLAGS_OFFSET = 12;
constexpr std::size_t ENTRY_PAYLOAD_OFFSET = 16;
constexpr std::size_t ENTRY_SIZE_OFFSET = 24;
constexpr std::size_t ENTRY_CRC_OFFSET = 32;
constexpr std::size_t ENTRY_RESERVED_OFFSET = 36;
constexpr std::uint16_t VERSION_MAJOR = 1;
constexpr std::uint16_t VERSION_MINOR = 0;
constexpr std::uint32_t MAX_ENTRY_COUNT = 104857;
constexpr std::uint64_t MAX_TOC_SIZE = 4ULL * 1024ULL * 1024ULL;
constexpr std::uint64_t MAX_FLATBUFFER_SIZE = 64ULL * 1024ULL * 1024ULL;
constexpr std::uint64_t MAX_PNG_SIZE = 64ULL * 1024ULL * 1024ULL;
constexpr std::uint64_t PAYLOAD_ALIGNMENT = 16;
constexpr std::uint64_t READ_CHUNK_SIZE = 64ULL * 1024ULL;
constexpr std::uint32_t INVALID_TYPE = 0;
constexpr std::uint32_t SHIFT_8 = 8;
constexpr std::uint32_t SHIFT_16 = 16;
constexpr std::uint32_t SHIFT_24 = 24;
constexpr std::size_t U64_BYTE_COUNT = 8;

constexpr std::array<std::uint8_t, 4> SIGNATURE = {'K', 'P', 'A', 'K'};

std::uint64_t AlignUp(std::uint64_t value, std::uint64_t alignment)
{
    return (value + alignment - 1U) & ~(alignment - 1U);
}

// El lector recorre un búfer contiguo ya validado; se usa acceso por índice
// para no convertir offsets del archivo en punteros a estructuras.
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
std::uint16_t ReadU16(const std::uint8_t *data, std::size_t offset)
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data[offset]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(data[offset + 1])
                                   << SHIFT_8));
}

std::uint32_t ReadU32(const std::uint8_t *data, std::size_t offset)
{
    return static_cast<std::uint32_t>(data[offset]) |
           (static_cast<std::uint32_t>(data[offset + 1]) << SHIFT_8) |
           (static_cast<std::uint32_t>(data[offset + 2]) << SHIFT_16) |
           (static_cast<std::uint32_t>(data[offset + 3]) << SHIFT_24);
}

std::uint64_t ReadU64(const std::uint8_t *data, std::size_t offset)
{
    std::uint64_t value = 0;
    for (std::size_t index = 0; index < U64_BYTE_COUNT; ++index)
    {
        value |= static_cast<std::uint64_t>(data[offset + index]) << (SHIFT_8 * index);
    }
    return value;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

bool ReadExact(std::ifstream &file, void *data, std::size_t size)
{
    if (size == 0)
    {
        return true;
    }

    file.read(static_cast<char *>(data), static_cast<std::streamsize>(size));
    return static_cast<bool>(file) && file.gcount() == static_cast<std::streamsize>(size);
}

std::uint64_t MaxSizeForType(PakResourceType type)
{
    switch (type)
    {
    case PakResourceType::LEVEL:
    case PakResourceType::ANIMATION:
        return MAX_FLATBUFFER_SIZE;
    case PakResourceType::PNG:
        return MAX_PNG_SIZE;
    }

    return 0;
}

const char *TypeName(PakResourceType type)
{
    switch (type)
    {
    case PakResourceType::LEVEL:
        return "Level";
    case PakResourceType::ANIMATION:
        return "Animation";
    case PakResourceType::PNG:
        return "PNG";
    }

    return "desconocido";
}

std::string HexValue(std::uint32_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << value;
    return stream.str();
}

} // namespace

bool PakReader::Open(const std::string &path)
{
    Close();
    _path = path;
    _file.open(_path, std::ios::binary);
    if (!_file)
    {
        return fail("No se pudo abrir el paquete: " + _path);
    }

    if (!read_file_size() || !read_header_and_toc())
    {
        _file.close();
        _entries.clear();
        return false;
    }

    return true;
}

void PakReader::Close() noexcept
{
    if (_file.is_open())
    {
        _file.close();
    }

    _path.clear();
    _last_error.clear();
    _file_size = 0;
    _data_start = 0;
    _entries.clear();
}

const std::vector<PakEntry> &PakReader::Entries() const noexcept
{
    return _entries;
}

const std::string &PakReader::LastError() const noexcept
{
    return _last_error;
}

const std::string &PakReader::Path() const noexcept
{
    return _path;
}

bool PakReader::fail(std::string message) const
{
    _last_error = std::move(message);
    return false;
}

bool PakReader::read_file_size()
{
    _file.seekg(0, std::ios::end);
    const auto END_POSITION = _file.tellg();
    if (END_POSITION == std::streampos(-1))
    {
        return fail("No se pudo determinar el tamaño del paquete: " + _path);
    }

    _file_size = static_cast<std::uint64_t>(END_POSITION);
    _file.seekg(0, std::ios::beg);
    if (!_file)
    {
        return fail("No se pudo volver al inicio del paquete: " + _path);
    }

    if (_file_size < HEADER_SIZE)
    {
        return fail("El paquete es más pequeño que la cabecera: " + _path);
    }

    return true;
}

bool PakReader::read_header_and_toc()
{
    std::array<std::uint8_t, HEADER_SIZE> header{};
    if (!ReadExact(_file, header.data(), header.size()))
    {
        return fail("No se pudo leer la cabecera del paquete: " + _path);
    }

    if (!std::equal(SIGNATURE.begin(), SIGNATURE.end(), header.begin()))
    {
        return fail("Firma incorrecta en el paquete: " + _path);
    }

    const std::uint16_t MAJOR = ReadU16(header.data(), HEADER_VERSION_MAJOR_OFFSET);
    const std::uint16_t MINOR = ReadU16(header.data(), HEADER_VERSION_MINOR_OFFSET);
    if (MAJOR != VERSION_MAJOR || MINOR != VERSION_MINOR)
    {
        return fail("Versión no soportada en " + _path + ": " + std::to_string(MAJOR) +
                    "." + std::to_string(MINOR) + " (se esperaba 1.0).");
    }

    const std::uint32_t COUNT = ReadU32(header.data(), HEADER_COUNT_OFFSET);
    const std::uint32_t EXPECTED_TOC_CRC = ReadU32(header.data(), HEADER_TOC_CRC_OFFSET);

    if (COUNT > MAX_ENTRY_COUNT)
    {
        return fail("Cantidad de entradas fuera de límite en " + _path + ": " +
                    std::to_string(COUNT) + " (máximo " +
                    std::to_string(MAX_ENTRY_COUNT) + ").");
    }

    const std::uint64_t TOC_SIZE =
        static_cast<std::uint64_t>(COUNT) * static_cast<std::uint64_t>(TOC_ENTRY_SIZE);
    if (TOC_SIZE > MAX_TOC_SIZE)
    {
        return fail("La tabla de contenidos supera 4 MiB en " + _path + ".");
    }

    const std::uint64_t TOC_END = static_cast<std::uint64_t>(HEADER_SIZE) + TOC_SIZE;
    if (TOC_END > _file_size)
    {
        return fail("Tabla de contenidos truncada en " + _path + ".");
    }

    std::vector<std::uint8_t> toc(static_cast<std::size_t>(TOC_SIZE));
    if (!toc.empty() && !ReadExact(_file, toc.data(), toc.size()))
    {
        return fail("No se pudo leer la tabla de contenidos de " + _path + ".");
    }

    const std::uint32_t ACTUAL_TOC_CRC = ComputeCrc32(toc.data(), toc.size());
    if (ACTUAL_TOC_CRC != EXPECTED_TOC_CRC)
    {
        return fail("CRC de tabla incorrecto en " + _path + ": esperado " +
                    HexValue(EXPECTED_TOC_CRC) + ", calculado " +
                    HexValue(ACTUAL_TOC_CRC) + ".");
    }

    const std::uint64_t DATA_START = AlignUp(TOC_END, PAYLOAD_ALIGNMENT);
    if (!parse_and_validate_entries(toc, COUNT, DATA_START))
    {
        return false;
    }

    _data_start = DATA_START;
    return true;
}

// La validación es una secuencia deliberada de comprobaciones del contrato.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool PakReader::parse_and_validate_entries(
    const std::vector<std::uint8_t> &toc, // NOLINT(bugprone-easily-swappable-parameters)
    std::uint32_t count,                  // NOLINT(bugprone-easily-swappable-parameters)
    std::uint64_t data_start)
{
    std::vector<PakEntry> entries;
    entries.reserve(static_cast<std::size_t>(count));

    std::uint64_t previous_id = 0;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        const std::size_t BASE = static_cast<std::size_t>(index) * TOC_ENTRY_SIZE;
        const std::uint8_t *const ENTRY_DATA = toc.data();

        PakEntry entry{};
        entry.id = ReadU64(ENTRY_DATA, BASE + ENTRY_ID_OFFSET);
        const std::uint32_t TYPE = ReadU32(ENTRY_DATA, BASE + ENTRY_TYPE_OFFSET);
        const std::uint32_t FLAGS = ReadU32(ENTRY_DATA, BASE + ENTRY_FLAGS_OFFSET);
        entry.offset = ReadU64(ENTRY_DATA, BASE + ENTRY_PAYLOAD_OFFSET);
        entry.size = ReadU64(ENTRY_DATA, BASE + ENTRY_SIZE_OFFSET);
        entry.crc32 = ReadU32(ENTRY_DATA, BASE + ENTRY_CRC_OFFSET);
        const std::uint32_t RESERVED = ReadU32(ENTRY_DATA, BASE + ENTRY_RESERVED_OFFSET);

        if (TYPE == INVALID_TYPE ||
            TYPE > static_cast<std::uint32_t>(PakResourceType::PNG))
        {
            return fail("Tipo de recurso inválido en " + _path + " (ID " +
                        std::to_string(entry.id) + ", tipo " + std::to_string(TYPE) +
                        ").");
        }

        entry.type = static_cast<PakResourceType>(TYPE);

        if (FLAGS != 0U)
        {
            return fail("Flags distintos de cero en " + _path + " (ID " +
                        std::to_string(entry.id) + ").");
        }

        if (RESERVED != 0U)
        {
            return fail("Campo reservado distinto de cero en " + _path + " (ID " +
                        std::to_string(entry.id) + ").");
        }

        if (entry.id == 0)
        {
            return fail("ID cero en la tabla de " + _path + ".");
        }

        if (entry.id == previous_id)
        {
            return fail("ID duplicado en " + _path + ": " + std::to_string(entry.id) +
                        ".");
        }

        if (entry.id < previous_id)
        {
            return fail("IDs desordenados en " + _path + " (ID " +
                        std::to_string(entry.id) + " después de " +
                        std::to_string(previous_id) + ").");
        }
        previous_id = entry.id;

        if ((entry.offset % PAYLOAD_ALIGNMENT) != 0)
        {
            return fail("Offset no alineado a 16 bytes en " + _path + " (ID " +
                        std::to_string(entry.id) + ", offset " +
                        std::to_string(entry.offset) + ").");
        }

        if (entry.offset < data_start)
        {
            return fail("Offset dentro de la cabecera o la tabla en " + _path + " (ID " +
                        std::to_string(entry.id) + ", offset " +
                        std::to_string(entry.offset) + ").");
        }

        if (entry.size == 0)
        {
            return fail("Tamaño cero en " + _path + " (ID " + std::to_string(entry.id) +
                        ").");
        }

        const std::uint64_t TYPE_LIMIT = MaxSizeForType(entry.type);
        if (entry.size > TYPE_LIMIT)
        {
            return fail("Tamaño fuera de límite en " + _path + " (ID " +
                        std::to_string(entry.id) + ", tamaño " +
                        std::to_string(entry.size) + ", máximo " +
                        std::to_string(TYPE_LIMIT) + ").");
        }

        if (entry.offset > _file_size || entry.size > (_file_size - entry.offset))
        {
            return fail("Rango fuera del archivo en " + _path + " (ID " +
                        std::to_string(entry.id) + ", offset " +
                        std::to_string(entry.offset) + ", tamaño " +
                        std::to_string(entry.size) + ").");
        }

        entries.push_back(entry);
    }

    std::vector<const PakEntry *> ranges;
    ranges.reserve(entries.size());
    for (const PakEntry &entry : entries)
    {
        ranges.push_back(&entry);
    }

    std::sort(ranges.begin(), ranges.end(),
              [](const PakEntry *left, const PakEntry *right) {
                  return left->offset < right->offset;
              });

    for (std::size_t index = 1; index < ranges.size(); ++index)
    {
        const PakEntry &previous = *ranges[index - 1U];
        const PakEntry &current = *ranges[index];
        const std::uint64_t PREVIOUS_END = previous.offset + previous.size;
        if (current.offset < PREVIOUS_END)
        {
            return fail("Payloads solapados en " + _path + " (IDs " +
                        std::to_string(previous.id) + " y " + std::to_string(current.id) +
                        ").");
        }
    }

    _entries = std::move(entries);
    return true;
}

const PakEntry *PakReader::find_entry(std::uint64_t resource_id) const noexcept
{
    std::size_t low = 0;
    std::size_t high = _entries.size();

    while (low < high)
    {
        const std::size_t MIDDLE = low + ((high - low) / 2U);
        if (_entries.at(MIDDLE).id < resource_id)
        {
            low = MIDDLE + 1U;
        }
        else
        {
            high = MIDDLE;
        }
    }

    if (low < _entries.size() && _entries.at(low).id == resource_id)
    {
        return &_entries.at(low);
    }

    return nullptr;
}

bool PakReader::Read(std::uint64_t resource_id, PakResourceType expected_type,
                     std::vector<std::uint8_t> &out) const
{
    _last_error.clear();
    out.clear();

    if (!_file.is_open())
    {
        return fail("El paquete no está abierto.");
    }

    const PakEntry *const ENTRY = find_entry(resource_id);
    if (ENTRY == nullptr)
    {
        return fail("No existe el recurso " + std::to_string(resource_id) + " en " +
                    _path + ".");
    }

    if (ENTRY->type != expected_type)
    {
        return fail("Tipo inesperado para el recurso " + std::to_string(resource_id) +
                    " en " + _path + ": se esperaba " + TypeName(expected_type) +
                    " y se encontró " + TypeName(ENTRY->type) + ".");
    }

    if (ENTRY->size > std::numeric_limits<std::size_t>::max())
    {
        return fail("El recurso " + std::to_string(resource_id) +
                    " no cabe en memoria en este sistema.");
    }

    std::vector<std::uint8_t> data(static_cast<std::size_t>(ENTRY->size));
    _file.clear();
    _file.seekg(static_cast<std::streamoff>(ENTRY->offset), std::ios::beg);
    if (!_file)
    {
        return fail("No se pudo posicionar en el recurso " + std::to_string(resource_id) +
                    " de " + _path + ".");
    }

    Crc32 crc;
    std::uint64_t total_read = 0;
    while (total_read < ENTRY->size)
    {
        const std::uint64_t REMAINING = ENTRY->size - total_read;
        const auto CHUNK =
            static_cast<std::size_t>(std::min<std::uint64_t>(REMAINING, READ_CHUNK_SIZE));
        const auto OFFSET = static_cast<std::size_t>(total_read);

        if (!ReadExact(_file, &data[OFFSET], CHUNK))
        {
            out.clear();
            return fail("No se pudo leer el recurso " + std::to_string(resource_id) +
                        " de " + _path + " (offset " + std::to_string(ENTRY->offset) +
                        ", tamaño " + std::to_string(ENTRY->size) + ").");
        }

        crc.Update(&data[OFFSET], CHUNK);
        total_read += CHUNK;
    }

    const std::uint32_t ACTUAL_CRC = crc.Value();
    if (ACTUAL_CRC != ENTRY->crc32)
    {
        out.clear();
        return fail("CRC de recurso incorrecto en " + _path + " (ID " +
                    std::to_string(resource_id) + ", offset " +
                    std::to_string(ENTRY->offset) + ", tamaño " +
                    std::to_string(ENTRY->size) + "): esperado " +
                    HexValue(ENTRY->crc32) + ", calculado " + HexValue(ACTUAL_CRC) + ".");
    }

    out = std::move(data);
    return true;
}

} // namespace k_engine::resource
