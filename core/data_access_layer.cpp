#include "data_access_layer.h"
#include <cstdint>

namespace toocal::core::data_access_layer
{
  [[nodiscard]] auto Data_access_layer::get_system_page_size() noexcept -> uint32_t
  {
#ifdef __unix__
    return getpagesize();
#endif

// Not tested yet
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<uint32_t>(sysInfo.dwPageSize);
#endif
  }

  [[nodiscard]] auto Data_access_layer::intialize_database() noexcept
    -> tl::expected<Data_access_layer, Error>
  {}

  [[nodiscard]] auto
    Data_access_layer::load_database() noexcept -> tl::expected<Data_access_layer, Error>
  {}

  auto Data_access_layer::close() noexcept -> void { this->file.close(); }
} // namespace toocal::core::data_access_layer