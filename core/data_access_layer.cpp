#include "data_access_layer.h"
#include "errors.hpp"
#include "page.h"
#include <cerrno>
#include <cstring>
#include "tl/expected.hpp"
#include <cstdint>
#include <exception>
#include <vector>

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

  [[nodiscard]] auto
    Data_access_layer::allocate_empty_page(page::Page_num page_num) const noexcept -> Page
  {
    return Page{page_num, std::vector<uint8_t>(Data_access_layer::DEFAULT_PAGE_SIZE)};
  }

  [[nodiscard]] auto Data_access_layer::write_page(const Page &page) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    try
      {
        if (this->file.seekp(page.page_num * Data_access_layer::DEFAULT_PAGE_SIZE).fail())
          return Err(std::strerror(errno));

        if (this->file
              .write(reinterpret_cast<const char *>(page.data.data()), page.data.size())
              .fail())
          return Err(std::strerror(errno));

        return nullptr;
      }
    catch (const std::exception &e)
      {
        return Err(e.what());
      }
  }

  [[nodiscard]] auto Data_access_layer::read_page(page::Page_num page_num) noexcept
    -> tl::expected<Page, Error>
  {
    try
      {
        auto page = this->allocate_empty_page(page_num);
        if (this->file.seekg(page_num * Data_access_layer::DEFAULT_PAGE_SIZE).fail())
          return Err(std::strerror(errno));

        if (this->file.read(reinterpret_cast<char *>(page.data.data()), page.data.size())
              .fail())
          return Err(std::strerror(errno));

        return page;
      }
    catch (const std::exception &e)
      {
        return Err(e.what());
      }
  }
} // namespace toocal::core::data_access_layer