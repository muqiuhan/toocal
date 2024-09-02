#ifndef TOOCAL_CORE_FREELIST_H
#define TOOCAL_CORE_FREELIST_H

#include "./page.h"
#include "errors.hpp"
#include "types.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <endian/stream_reader.hpp>
#include <endian/stream_writer.hpp>
#include <endian/little_endian.hpp>

namespace toocal::core::freelist
{
  /** Freelist manages the manages free and used pages. */
  class Freelist
  {
  public:
    /** META_PAGE is the maximum page num that is used by the db for its own purposes.
     ** For now, only page 0 is used as the header page.
     ** It means all other page numbers can be used. */
    inline static const page::Page_num META_PAGE = 0;

  public:
    /** max_page holds the latest page num allocated. */
    page::Page_num max_page;

    /** released_pages holds all the ids that were released during delete.
     ** New page ids are first given from the releasedPageIDs to avoid growing the file.
     ** If it's empty, then max_page is incremented and a new page is created thus
     ** increasing the file size. */
    std::vector<page::Page_num> released_pages;

  public:
    /** get_next_page returns page ids for writing New page ids are first given from the
     ** releasedPageIDs to avoid growing the file. If it's empty, then maxPage is
     ** incremented and a new page is created thus increasing the file size. */
    [[nodiscard]] auto get_next_page() noexcept -> page::Page_num;
    auto               release_page(page::Page_num page) noexcept -> void;
  };
}; // namespace toocal::core::freelist

namespace toocal::core::types
{
  using errors::Error;
  using freelist::Freelist;

  template <> class Serializer<Freelist>
  {
  public:
    [[nodiscard]] static auto serialize(const Freelist &self) noexcept
      -> tl::expected<std::vector<std::uint8_t>, Error>
    {
      const auto max_page = static_cast<uint16_t>(self.max_page),
                 released_pages_count =
                   static_cast<uint16_t>(self.released_pages.size());

      auto buffer = std::vector<uint8_t>(
        /* max_page */
        sizeof(max_page) +
        /* released pages count */
        sizeof(released_pages_count) +
        /* released pages */
        (sizeof(page::Page_num) * self.released_pages.size()));

      auto serializer = endian::stream_writer<endian::little_endian>(
        buffer.data(), buffer.size());

      serializer << max_page << released_pages_count;

      std::ranges::for_each(
        self.released_pages,
        [&](const auto &released_page) { serializer << released_page; });

      return buffer;
    }

    [[nodiscard]] static auto
      deserialize(const std::vector<std::uint8_t> &buffer) noexcept
      -> tl::expected<Freelist, Error>
    {
      auto deserializer = endian::stream_reader<endian::little_endian>(
        buffer.data(), buffer.size());

      uint16_t max_page = 0;
      uint16_t released_pages_count = 0;

      deserializer >> max_page >> released_pages_count;

      std::vector<page::Page_num> released_pages(released_pages_count);
      for (uint16_t i = 0; i < released_pages_count; i++)
        deserializer >> released_pages[i];

      return Freelist{static_cast<page::Page_num>(max_page), released_pages};
    }
  };
} // namespace toocal::core::types

#endif /* TOOCAL_CORE_FREELIST_H */