#include "utils.h"
#include "errors.hpp"

#ifdef __unix__
#include <sys/stat.h>
#endif
namespace toocal::core::utils
{
  [[nodiscard]] auto Safecmp::memcmp(const std::string& k1, const std::string& k2) noexcept -> int
  {
    return k1.size() == k2.size()
             ? std::memcmp(k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()))
           : k1.size() > k2.size() ? 1
                                   : -1;
  }

  [[nodiscard]] auto Safecmp::bytescmp(
    const std::vector<uint8_t>& k1, const std::vector<uint8_t>& k2) noexcept -> int
  {
    return k1.size() == k2.size()
             ? std::memcmp(k1.data(), k2.data(), (k1.size() > k2.size() ? k2.size() : k1.size()))
           : k1.size() > k2.size() ? 1
                                   : -1;
  }

  [[nodiscard]] auto Filesystem::sizeof_file(const std::string& path) noexcept -> size_t
  {
#ifdef __unix__
    struct stat fileStat;

    if (0 > stat(path.c_str(), &fileStat))
      {
        spdlog::error("Error getting {} size: {}", path, strerror(errno));
        fatal(fmt::format("Error getting {} size: {}", path, strerror(errno)));
      }

    return static_cast<size_t>(fileStat.st_size) / (1024 * 1024);
#else
    unimplemented();
#endif
  }
} // namespace toocal::core::utils