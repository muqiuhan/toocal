#ifndef TOOCAL_CORE_UTILS_H
#define TOOCAL_CORE_UTILS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <tl/expected.hpp>

namespace toocal::core::utils
{
  class Safecmp
  {
  public:
    [[nodiscard]] static auto memcmp(const std::string& k1, const std::string& k2) noexcept -> int;
    [[nodiscard]] static auto
      bytescmp(const std::vector<uint8_t>& k1, const std::vector<uint8_t>& k2) noexcept -> int;
  };

  class Filesystem
  {
  public:
    [[nodiscard]] static auto sizeof_file(const std::string& path) noexcept -> size_t;
  };
} // namespace toocal::core::utils

#endif /* TOOCAL_CORE_UTILS_H */
