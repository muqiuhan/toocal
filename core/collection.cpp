#include "collection.h"
#include "data_access_layer.h"

namespace toocal::core::collection
{
  [[nodiscard]] auto Collection::find(const std::vector<uint8_t> key)
    const noexcept -> tl::expected<tl::optional<node::Item>, Error>
  {
    return this->dal->get_node(this->root)
      .and_then([&](auto && node) { return node.find_key(key, true); })
      .map([&](const auto && result) -> tl::optional<node::Item> {
        const auto [index, containing_node, _] = result;
        if (-1 == index)
          return tl::nullopt;
        else
          return containing_node->items[index];
      });
  }
} // namespace toocal::core::collection