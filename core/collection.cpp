#include "collection.h"
#include "data_access_layer.h"
#include "node.h"
#include "page.h"
#include <cstring>
#include "utils.h"

namespace toocal::core::collection
{
  [[nodiscard]] auto Collection::find(std::vector<uint8_t> key) const noexcept
    -> tl::expected<tl::optional<node::Item>, Error>
  {
    return this->dal->get_node(this->root)
      .and_then([&](auto &&node) { return node.find_key(key, true); })
      .map([&](const auto &&result) -> tl::optional<node::Item> {
        const auto [index, containing_node, _] = result;
        if (-1 == index)
          return tl::nullopt;
        return containing_node->items[index];
      });
  }

  [[nodiscard]] auto Collection::put(std::vector<uint8_t> key, std::vector<uint8_t> value) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    const auto item = node::Item{std::move(key), std::move(value)};

    /* On first insertion the root node does not exist,
     * so it should be created. */
    Node root;

    if (0 == this->root)
      {
        root = std::move(this->dal->new_node(std::vector{item}, std::vector<page::Page_num>{}));

        return this->dal->write_node(root).map([&](const auto &&_) {
          this->root = root.page_num;
          return nullptr;
        });
      }

    this->dal->get_node(this->root)
      .map([&](const auto &exist_root) { root = exist_root; })
      .map_error([&](auto &&error) {
        error.append("get_node error in Collection::put");
        return error.panic();
      });

    /* Find the path to the node where the insertion should happen */
    auto [insertion_index, node_to_insertin, ancestors_indexes] =
      root.find_key(item.key, false)
        .map_error([&](auto &&error) {
          error.append("find_key error in Collection::put");
          return error.panic();
        })
        .value();

    /* If key already exists */
    if (
      node_to_insertin.has_value() && !node_to_insertin.value().items.empty()
      && insertion_index < node_to_insertin.value().items.size()
      && 0 == utils::Safecmp::bytescmp(node_to_insertin.value().items[insertion_index].key, key))
      node_to_insertin.value().items[insertion_index] = item;

    else /* Add item to the leaf node */
      node_to_insertin.value().add_item(item, insertion_index);

    node_to_insertin.value().dal->write_node(node_to_insertin.value()).map_error([&](auto &&error) {
      error.append("write_node error in Collection::put");
      return error.panic();
    });

    auto ancestors = this->get_nodes(ancestors_indexes)
                       .map_error([&](auto &&error) {
                         error.append("get_node error in Collection::put");
                         return error.panic();
                       })
                       .value();

    /* Rebalanced the nodes all the way up. Start From one node before
     * the last and go all the way up. Exclude root. */
    for (auto i = static_cast<int64_t>(ancestors.size() - 2); i >= 0; i--)
      {
        auto previous_node = ancestors[i];

        if (auto node = ancestors[i + 1]; node.is_over_populated())
          previous_node.split(node, ancestors_indexes[i + 1]);
      }

    /* Handle root */
    if (auto &root_node = ancestors[0]; root_node.is_over_populated())
      {
        auto new_root = this->dal->new_node({}, {root_node.page_num});
        new_root.split(root_node, 0);

        /* commit newly created root */
        this->dal->write_node(new_root)
          .map([&](const auto &&_) { this->root = new_root.page_num; })
          .map_error([&](auto &&error) {
            error.append("write_node error in Collection::put");
            return error.panic();
          });
      }

    return nullptr;
  }

  [[nodiscard]] auto Collection::get_nodes(std::vector<uint32_t> indexes) const noexcept
    -> tl::expected<std::vector<Node>, Error>
  {
    return this->dal->get_node(this->root).map([&](const auto &&root) {
      auto nodes = std::vector<Node>{root};
      auto child = root;

      for (uint32_t i = 1; i < indexes.size(); i++)
        this->dal->get_node(child.children[indexes[i]])
          .map([&](const auto &&new_child) {
            child = new_child;
            nodes.push_back(child);
          })
          .map_error([&](auto &&error) {
            error.append("get_node error in Collection::get_nodes");
            return error.panic();
          });

      return nodes;
    });
  }

} // namespace toocal::core::collection