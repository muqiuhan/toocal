#include "utils.h"

namespace toocal::core::utils
{
  [[nodiscard]] auto safe_memcmp(std::string k1, std::string k2) noexcept -> int
  {
    return std::memcmp(
      k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()));
  }

  [[nodiscard]] auto safe_bytescmp(
    std::vector<uint8_t> k1, std::vector<uint8_t> k2) noexcept -> int
  {
    return std::memcmp(
      k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()));
  }
} // namespace toocal::core::utils