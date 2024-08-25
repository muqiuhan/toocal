#ifndef TOOCAL_CORE_FREELIST_H
#define TOOCAL_CORE_FREELIST_H

#include "./page.h"
#include "types.hpp"
#include <vector>

namespace toocal::core::freelist
{
  class Freelist
  {
  public:
    inline static const page::Page_num META_PAGE = 0;

  private:
    page::Page_num              max_page;
    std::vector<page::Page_num> released_pages;

  public:
    [[nodiscard]] auto get_next_page() noexcept -> page::Page_num;
    auto               release_page(page::Page_num page) noexcept -> void;
  };
}; // namespace toocal::core::freelist

namespace toocal::core::types
{
  template <> class Serializer<freelist::Freelist>
  {
  public:
    [[nodiscard]] auto serialize(const freelist::Freelist &self) const noexcept
      -> tl::expected<std::vector<std::byte>, errors::Error>
    {
      unimplemented();
    }

    [[nodiscard]] auto deserialize(const std::vector<std::byte> &bytes) const noexcept
      -> tl::expected<freelist::Freelist, errors::Error>
    {
      unimplemented();
    }
  };
} // namespace toocal::core::types

#endif /* TOOCAL_CORE_FREELIST_H */