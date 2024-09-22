#ifndef TOOCAL_CORE_PAGE_H
#define TOOCAL_CORE_PAGE_H

#include <cstdint>
#include <vector>

namespace toocal::core::page
{
  using Page_num = uint64_t;

  class Page
  {
  public:
    Page_num                  page_num;
    std::vector<std::uint8_t> data;

  private:
    /** Get the virtual memory page size of the current operating system.
     ** Currently, only the POSIX standard is supported.
     ** TODO: Windows support. */
    [[nodiscard]] static auto get_system_page_size() noexcept -> uint32_t;

  public:
    inline static const auto DEFAULT_PAGE_SIZE = get_system_page_size();
  };
} // namespace toocal::core::page

#endif /* TOOCAL_CORE_PAGE_H */
