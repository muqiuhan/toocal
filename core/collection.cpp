#include "collection.h"
#include "data_access_layer.h"
#include "errors.hpp"
#include "node.h"
#include "page.h"
#include <cstring>
#include "tl/expected.hpp"
#include "utils.h"

namespace toocal::core::collection
{
  [[nodiscard]] auto Collection::find(std::vector<uint8_t> key) const noexcept
    -> tl::expected<tl::optional<node::Item>, Error>
  {
    return this->dal->get_node(this->root)
      .and_then([&](const auto &node) { return node.find_key(key, true); })
      .map([&](const auto &result) -> tl::optional<node::Item> {
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
        root = std::move(this->dal->new_node(std::deque{item}, std::deque<page::Page_num>{}));

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
    return root.find_key(item.key, false).and_then([&](auto &&result) {
      auto &[insertion_index, node_to_insertin, ancestors_indexes] = result;

      /* If key already exists */
      if (
        node_to_insertin.has_value() && !node_to_insertin->items.empty()
        && 0 == utils::Safecmp::bytescmp(node_to_insertin.value().items[insertion_index].key, key))
        node_to_insertin->items.insert(node_to_insertin->items.begin() + insertion_index, item);

      else /* Add item to the leaf node */
        node_to_insertin.value().add_item(item, insertion_index);

      node_to_insertin.value().dal->write_node(node_to_insertin.value()).map_error([&](auto &&error) {
        error.append("write_node error in Collection::put");
        return error.panic();
      });

      return this->get_nodes(ancestors_indexes).map([&](auto &&ancestors) {
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
      });
    });
  }

  [[nodiscard]] auto Collection::get_nodes(std::deque<uint32_t> indexes) const noexcept
    -> tl::expected<std::deque<Node>, Error>
  {
    return this->dal->get_node(this->root).map([&](const auto &root) {
      auto nodes = std::deque<Node>{root};
      auto child = root;

      for (uint32_t i = 1; i < indexes.size(); i++)
        this->dal->get_node(child.children[indexes[i]])
          .map([&](auto &&new_child) {
            child = std::move(new_child);
            nodes.push_back(child);
          })
          .map_error([&](auto &&error) {
            error.append("get_node error in Collection::get_nodes");
            return error.panic();
          });

      return nodes;
    });
  }

  [[nodiscard]] auto Collection::remove(const std::vector<uint8_t> &key) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    return this->dal->get_node(this->root).and_then([&](auto &&root) {
      return root.find_key(key, true).and_then(
        [&](auto &&result) -> tl::expected<std::nullptr_t, Error> {
          auto &[remove_item_index, node_to_remove_from, ancestors_indexes] = result;

          if (remove_item_index == -1)
            return tl::unexpected(_error(fmt::format(
              "key {} not found in Collection::remove", std::string{key.begin(), key.end()})));

          if (node_to_remove_from->is_leaf())
            node_to_remove_from->remove_item_from_leaf(remove_item_index);
          else
            {
              node_to_remove_from->remove_item_from_internal(remove_item_index)
                .map([&](const auto &affected_nodes) {
                  return ancestors_indexes.insert(
                    ancestors_indexes.end(), affected_nodes.begin(), affected_nodes.end());
                })
                .map_error([&](auto &&error) {
                  error.append("remove_item_from_internal error in Collection::remove");
                  return error.panic();
                });
            }

          return this->get_nodes(ancestors_indexes).map([&](auto &&ancestors) {
            /* Rebalance the nodes all the way up.
             * Start From one node before the last and go all the way up. Exclude root. */
            for (auto i = static_cast<int64_t>(ancestors.size() - 2); i >= 0; i--)
              {
                auto pnode = ancestors[i];
                auto node = ancestors[i + 1];

                if (node.is_under_populated())
                  pnode.rebalance_remove(node, ancestors_indexes[i + 1])
                    .map_error([&](auto &&error) {
                      error.append("rebalance_remove error in Collection::remove");
                      return error.panic();
                    });
              }

            root = ancestors[0];

            /* If the root has no items after rebalancing,
             * there's no need to save it because we ignore it. */
            if (root.items.empty() && !root.children.empty())
              this->root = ancestors[1].page_num;

            return nullptr;
          });
        });
    }).and_then([&](const auto &) -> tl::expected<std::nullptr_t, Error> {
      this->find(key).and_then([&](const auto &item) {
        while (item == tl::nullopt)
          {
            spdlog::warn("failed to remove key: {}, retrying...", std::string{key.begin(), key.end()});
            this->remove(key);
          }
        return tl::expected<std::nullptr_t, Error>(nullptr);
      });
    });
  }

} // namespace toocal::core::collection
