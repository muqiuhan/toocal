#ifndef TOOCAL_CORE_COLLECTION_H
#define TOOCAL_CORE_COLLECTION_H

#include "errors.hpp"
#include "node.h"
#include "page.h"
#include "tl/expected.hpp"
#include "tl/optional.hpp"
#include <cstdint>
#include <vector>

namespace toocal::core::data_access_layer
{
  class Data_access_layer;
}

namespace toocal::core::collection
{
  using errors::Error;

  class Collection
  {
  public:
    data_access_layer::Data_access_layer * dal;
    std::vector<uint8_t>                   name;
    page::Page_num                         root;

  public:
    Collection(std::vector<uint8_t> name, page::Page_num root)
      : name(std::move(name)), root(root)
    {}

    /** Returns an item according based on the given key by performing a
     ** binary search. */
    [[nodiscard]] auto find(const std::vector<uint8_t> key) const noexcept
      -> tl::expected<tl::optional<node::Item>, Error>;
  };
} // namespace toocal::core::collection

#endif /* collection.h */