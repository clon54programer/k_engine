#pragma once

#include <cstddef>
#include <cstdint>

namespace k_engine::resource
{

// CRC-32/ISO-HDLC incremental. Se usa para la tabla y los payloads del .pak.
class Crc32
{
  public:
    void Update(const std::uint8_t *data, std::size_t size) noexcept;
    [[nodiscard]] std::uint32_t Value() const noexcept;

  private:
    std::uint32_t _state = 0xFFFFFFFFU;
};

std::uint32_t ComputeCrc32(const std::uint8_t *data, std::size_t size) noexcept;

} // namespace k_engine::resource
