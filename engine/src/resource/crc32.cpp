#include "resource/crc32.h"

#include <array>

namespace k_engine::resource
{

namespace
{

constexpr std::uint32_t CRC32_POLYNOMIAL = 0xEDB88320U;
constexpr std::uint32_t BYTE_MASK = 0xFFU;
constexpr std::uint32_t BITS_PER_BYTE = 8U;
constexpr std::uint32_t CRC32_FINAL_XOR = 0xFFFFFFFFU;
constexpr std::size_t CRC_TABLE_SIZE = 256;

constexpr std::array<std::uint32_t, CRC_TABLE_SIZE> MakeCrcTable() noexcept
{
    std::array<std::uint32_t, CRC_TABLE_SIZE> table{};

    for (std::size_t index = 0; index < table.size(); ++index)
    {
        auto value = static_cast<std::uint32_t>(index);
        for (std::uint32_t bit = 0; bit < BITS_PER_BYTE; ++bit)
        {
            value =
                ((value & 1U) != 0U) ? (CRC32_POLYNOMIAL ^ (value >> 1U)) : (value >> 1U);
        }
        table.at(index) = value;
    }

    return table;
}

constexpr std::array<std::uint32_t, CRC_TABLE_SIZE> CRC_TABLE = MakeCrcTable();

} // namespace

void Crc32::Update(const std::uint8_t *data, std::size_t size) noexcept
{
    if (data == nullptr || size == 0)
    {
        return;
    }

    for (std::size_t index = 0; index < size; ++index)
    {
        // data es un búfer contiguo del tamaño indicado; el acceso se comprueba
        // con el bucle y no se convierte en un offset arbitrario.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const std::uint32_t INDEX = (_state ^ data[index]) & BYTE_MASK;
        _state = CRC_TABLE.at(INDEX) ^ (_state >> BITS_PER_BYTE);
    }
}

std::uint32_t Crc32::Value() const noexcept
{
    return _state ^ CRC32_FINAL_XOR;
}

std::uint32_t ComputeCrc32(const std::uint8_t *data, std::size_t size) noexcept
{
    Crc32 crc;
    crc.Update(data, size);
    return crc.Value();
}

} // namespace k_engine::resource
