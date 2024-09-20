#ifndef TOOCAL_CORE_COLLECTION_H
#define TOOCAL_CORE_COLLECTION_H

#include "errors.hpp"
#include "node.h"
#include "page.h"
#include "tl/expected.hpp"
#include "tl/optional.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace toocal::core::data_access_layer
{
  class Data_access_layer;
}

namespace toocal::core::collection
{
  using errors::Error;
  using node::Node;

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
    [[nodiscard]] auto find(std::vector<uint8_t> key) const noexcept
      -> tl::expected<tl::optional<node::Item>, Error>;

    /** Put adds a key to the tree. It finds the correct node and the insertion
     ** index and adds the item. When performing the search, the ancestors are
     ** returned as well. This way we can iterate over them to check which nodes
     ** were modified and rebalance by splitting them accordingly. If the root
     ** has too many items, then a new root of a new layer is created and the
     ** created nodes from the split are added as children. */
    [[nodiscard]] auto
      put(std::vector<uint8_t> key, std::vector<uint8_t> value) noexcept
      -> tl::expected<std::nullptr_t, Error>;

    /** Returns a list of nodes based on their indexes (the breadcrumbs from the
     ** root):
     **            p
     **        /       \
     **      a          b
     **   /     \     /   \
     **  c       d   e     f
     **  For [0,1,0] -> p,b,e    */
    [[nodiscard]] auto get_nodes(std::vector<uint32_t> indexes) const noexcept
      -> tl::expected<std::vector<Node>, Error>;
  };
} // namespace toocal::core::collection

#endif /* collection.h */