#ifndef TOOCAL_CORE_META_H
#define TOOCAL_CORE_META_H

#include "endian/little_endian.hpp"
#include "endian/stream_reader.hpp"
#include "endian/stream_writer.hpp"
#include "page.h"
#include "types.hpp"
#include <cstdint>
#include <sys/types.h>

namespace toocal::core::meta
{
  /** meta is the meta page of the db. */
  class Meta
  {
  public:
    /** The database has a root collection that holds all the collections in the database.
     ** It is called root and the root property of meta holds page number containing the
     ** root of collections collection. The keys are the collections names and the values
     ** are the page number of the root of each collection. Then, once the collection and
     ** *the root page are located, a search inside a collection can be made. */
    page::Page_num root;
    page::Page_num freelist_page;
  };
} // namespace toocal::core::meta

namespace toocal::core::types
{
  template <> class Serializer<meta::Meta>
  {
  public:
    [[nodiscard]] static auto serialize(const meta::Meta &self) noexcept
      -> tl::expected<std::vector<std::uint8_t>, errors::Error>
    {
      auto buffer = std::vector<uint8_t>(
        /* root */
        sizeof(self.root) +
        /* freelist_page */
        sizeof(self.freelist_page));

      auto serializer =
        endian::stream_writer<endian::little_endian>(buffer.data(), buffer.size());

      serializer << self.root << self.freelist_page;
      return buffer;
    }

    [[nodiscard]] static auto deserialize(const std::vector<std::uint8_t> &buffer) noexcept
      -> tl::expected<meta::Meta, errors::Error>
    {
      auto deserializer =
        endian::stream_reader<endian::little_endian>(buffer.data(), buffer.size());
      auto meta = meta::Meta{};

      deserializer >> meta.root >> meta.freelist_page;
      return meta;
    }
  };
} // namespace toocal::core::types

#endif /* TOOCAL_CORE_META_H */