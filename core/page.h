#ifndef TOOCAL_CORE_PAGE_H
#define TOOCAL_CORE_PAGE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace toocal::core::page
{
  using Page_num = uint64_t;

  class Page
  {
  public:
    const Page_num page_num;
    const std::vector<std::byte> data;
  };
} // namespace toocal::core::page

#endif /* TOOCAL_CORE_PAGE_H */