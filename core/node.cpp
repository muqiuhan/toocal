#include "node.h"
#include "errors.hpp"
#include "page.h"
#include "data_access_layer.h"
#include <cstdarg>
#include <cstddef>
#include <numeric>
#include <span>
#include <stdexcept>
#include "utils.h"

namespace toocal::core::node
{
  [[nodiscard]] auto Node::is_leaf() const noexcept -> bool { return this->children.empty(); }

  [[nodiscard]] auto Item::size() const noexcept -> uint32_t
  {
    return this->key.size() + this->value.size();
  }

  [[nodiscard]] auto Node::item_size(uint32_t index) const noexcept -> uint32_t
  {
    try
      {
        const auto& [key, value] = this->items[index];
        return key.size() + value.size() + sizeof(Page::page_num);
      }
    catch (const std::out_of_range& e)
      {
        fatal(fmt::format("cannot get the item size of {} because out of range", index));
      }
  }

  [[nodiscard]] auto Node::size() const noexcept -> uint32_t
  {
    return std::accumulate(
      items.begin(),
      items.end(),
      static_cast<uint32_t>(Node::HEADER_SIZE + sizeof(page::Page_num)),
      [&](const uint32_t size, const auto& item) { return size + item.size(); });
  }

  [[nodiscard]] auto Node::is_last(const uint32_t index, const Node& parent_node) const noexcept
    -> bool
  {
    return index == parent_node.items.size();
  }

  [[nodiscard]] auto Node::is_first(const uint32_t index) const noexcept -> bool
  {
    return index == 0;
  }

  [[nodiscard]] auto Node::find_key_helper(
    const Node&                 node,
    const std::vector<uint8_t>& key,
    const bool                  exact,
    std::deque<uint32_t>&       ancestors_indexes) noexcept
    -> tl::expected<std::tuple<int, tl::optional<Node>>, Error>
  {
    const auto&& [was_found, index] = node.find_key_in_node(key);
    if (was_found)
      return std::make_tuple(index, node);

    if (node.is_leaf())
      {
        if (exact)
          return std::make_tuple(-1, tl::nullopt);
        return std::make_tuple(index, node);
      }

    ancestors_indexes.push_back(index);
    const auto next_child = node.dal->get_node(node.children[index]).map_error([](auto&& error) {
      error.append("get_node error in find_key_helper");
      return error;
    });

    if (next_child.has_value())
      return find_key_helper(next_child.value(), key, exact, ancestors_indexes);
    return tl::unexpected(next_child.error());
  }

  [[nodiscard]] auto Node::find_key_in_node(const std::vector<uint8_t>& key) const noexcept
    -> std::tuple<bool, uint32_t>
  {
    for (uint32_t index = 0; const auto& [existing_item_key, _] : this->items)
      {
        const auto compare_result = utils::Safecmp::bytescmp(existing_item_key, key);
        if (compare_result == 0)
          return {true, index};

        /* The key is bigger than the previous item, so it doesn't exist in the
         * node, but may exist in child nodes. */
        if (compare_result > 0)
          return {false, index};
        ++index;
      }

    /* The key is bigger than the previous item, so it doesn't exist in the
     * node, but may exist in child nodes. */
    return {false, this->items.size()};
  }

  [[nodiscard]] auto Node::find_key(const std::vector<uint8_t>& key, const bool exact) const noexcept
    -> tl::expected<std::tuple<int, tl::optional<Node>, std::deque<uint32_t>>, Error>
  {
    auto ancestors_indexes = std::deque<uint32_t>{/* index of root */ 0};

    return find_key_helper(*this, key, exact, ancestors_indexes).map([&](const auto& result) {
      const auto& [index, node] = result;
      return std::make_tuple(index, node, ancestors_indexes);
    });
  }

  auto Node::add_item(const Item& item, const uint32_t insertion_index) noexcept -> uint32_t
  {
    if (this->items.size() == insertion_index)
      {
        this->items.push_back(item);
        return insertion_index;
      }
    this->items.insert(this->items.begin() + insertion_index, item);
    return insertion_index;
  }

  [[nodiscard]] auto Node::is_over_populated() const noexcept -> bool
  {
    return this->dal->is_over_populated(*this);
  }

  [[nodiscard]] auto Node::is_under_populated() const noexcept -> bool
  {
    return this->dal->is_under_populated(*this);
  }

  static auto _split_when_node_is_leaf(
    Data_access_layer* dal, Node& node_to_split, const uint32_t split_index, Node& new_node) noexcept
    -> void
  {
    auto node = dal->new_node(
      std::deque<Item>{node_to_split.items.begin() + split_index + 1, node_to_split.items.end()},
      std::deque<page::Page_num>{});

    dal->write_node(node)
      .map([&](const auto&& _) {
        new_node = std::move(node);
        node_to_split.items =
          std::deque<Item>{node_to_split.items.begin(), node_to_split.items.begin() + split_index};

        return nullptr;
      })
      .map_error([&](auto&& error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

  static auto _split_when_node_is_not_leaf(
    Data_access_layer* dal, Node& node_to_split, const uint32_t split_index, Node& new_node) noexcept
    -> void
  {
    auto node = dal->new_node(
      std::deque<Item>{node_to_split.items.begin() + split_index + 1, node_to_split.items.end()},
      std::deque<page::Page_num>{
        node_to_split.children.begin() + split_index + 1, node_to_split.children.end()});

    dal->write_node(node)
      .map([&](const auto&& _) {
        new_node = std::move(node);
        node_to_split.items =
          std::deque<Item>{node_to_split.items.begin(), node_to_split.items.begin() + split_index};

        node_to_split.children = std::deque<page::Page_num>{
          node_to_split.children.begin(), node_to_split.children.begin() + split_index + 1};

        return nullptr;
      })
      .map_error([&](auto&& error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

  auto Node::split(Node& node_to_split, const uint32_t node_to_split_index) noexcept -> void
  {
    /* the first index where min amount of bytes to populate a page is archived.
     * then add 1 so it will be split one index after. */
    const auto split_index = node_to_split.dal->get_split_index(node_to_split);
    const auto middle_item = node_to_split.items[split_index];

    Node new_node;
    if (node_to_split.is_leaf())
      _split_when_node_is_leaf(this->dal, node_to_split, split_index, new_node);
    else
      _split_when_node_is_not_leaf(this->dal, node_to_split, split_index, new_node);

    this->add_item(middle_item, node_to_split_index);

    /* if middle of list, then move items forward. */
    if (this->children.size() == node_to_split_index + 1)
      this->children.push_back(new_node.page_num);
    else
      this->children.insert(this->children.begin() + node_to_split_index + 1, new_node.page_num);

    this->dal->write_node(*this)
      .and_then([&](const auto&& _) { return this->dal->write_node(node_to_split); })
      .map_error([](auto&& error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

  auto Node::remove_item_from_leaf(int32_t index) noexcept -> void
  {
    this->items.erase(this->items.begin() + index);
    this->dal->write_node(*this).map_error([&](auto&& error) {
      error.append("write_node error in Node::remove_items_from_leaf");
      return error.panic();
    });
  }

  [[nodiscard]] auto Node::remove_item_from_internal(int32_t index) noexcept
    -> tl::expected<std::deque<int32_t>, Error>
  {
    auto affected_nodes = std::deque<int32_t>{index};
    return this->dal->get_node(this->children[index])
      .map([&](auto&& node) {
        /* starting from its left child, descend to the rightmost descendant. */
        while (!node.is_leaf())
          {
            const auto traversing_index = this->children.size() - 1;
            node.dal->get_node(node.children[traversing_index])
              .map([&](const auto&& _) {
                affected_nodes.push_back(traversing_index);
                return nullptr;
              })
              .map_error([](auto&& error) {
                error.append("get_node error in Node::remote_item_from_internal");
                return error.panic();
              });
          }

        /* replace the item that should be removed with
         * the item before inorder which we just found. */
        node.items.pop_back();

        return this->dal->write_node(*this)
          .map([&](const auto&& _) { return this->dal->write_node(node); })
          .map_error([](auto&& error) {
            error.append("write_node error in Node::remote_item_from_internal");
            return error.panic();
          });
      })
      .map([&](const auto&& _) { return affected_nodes; });
  }

  auto Node::rotate_right(Node& anode, Node& pnode, Node& bnode, int32_t bnode_index) noexcept
    -> void
  {
    /* get last item and remove it */
    const auto anode_item = anode.items.back();
    anode.items.pop_back();

    /* get item from parent node and assign the aNodeItem item instead. */
    const auto pnode_item_index = (this->is_first(bnode_index)) ? 0 : bnode_index - 1;
    const auto pnode_item = pnode.items[pnode_item_index];

    pnode.items[pnode_item_index] = anode_item;

    /* assign parent item to b and make it first */
    bnode.items.push_front(pnode_item);

    /* if it's an inner leaf then move children as well. */
    if (!anode.is_leaf())
      {
        const auto child_to_shift = anode.children.back();
        anode.children.pop_back();
        bnode.children.push_front(child_to_shift);
      }

    return;
  }

  auto Node::rotate_left(Node& anode, Node& pnode, Node& bnode, int32_t bnode_index) noexcept
    -> void
  {
    /* get first item and remove it */
    const auto bnode_item = bnode.items[0];
    bnode.items.pop_front();

    /* get item from parent node and assign the bNodeItem item instead */
    const auto pnode_item_index =
      (this->is_last(bnode_index, pnode)) ? (pnode.items.size() - 1) : bnode_index;

    const auto pnode_item = pnode.items[pnode_item_index];
    pnode.items[pnode_item_index] = bnode_item;

    /* assign parent item to a and make it last */
    anode.items.push_back(pnode_item);

    /* if it's an inner leaf then move children as well. */
    if (!bnode.is_leaf())
      {
        const auto child_to_shift = bnode.children[0];
        bnode.children.pop_front();
        anode.children.push_back(child_to_shift);
      }

    return;
  }

  [[nodiscard]] auto Node::merge(Node& bnode, int32_t bnode_index) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    return this->dal->get_node(this->children[bnode_index - 1]).and_then([&](auto&& node) {
      /* take the item from the parent, remove it and add it to the unbalanced node */
      const auto pnode_item = this->items[bnode_index - 1];
      this->items.erase(this->items.begin() + bnode_index - 1);
      node.items.push_back(pnode_item);

      if (!node.is_leaf())
        node.children.insert(node.children.end(), bnode.children.begin(), bnode.children.end());

      return this->dal->write_node(node)
        .and_then([&](const auto&& _) { return this->dal->write_node(*this); })
        .map([&](const auto&& _) {
          this->dal->delete_node(bnode.page_num);
          return nullptr;
        });
    });
  }

  [[nodiscard]] auto
    Node::rebalance_remove(Node& unbalanced_node, int32_t unbalanced_node_index) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    auto pnode = *this;
    /* right rotate */
    if (unbalanced_node_index != 0)
      {
        this->dal->get_node(pnode.children[unbalanced_node_index - 1])
          .and_then([&](auto&& left_node) -> tl::expected<std::nullptr_t, Error> {
            if (left_node.can_spare_an_element())
              {
                rotate_right(left_node, pnode, unbalanced_node, unbalanced_node_index);
                return this->dal->write_node(left_node)
                  .and_then([&](const auto&& _) { return this->dal->write_node(pnode); })
                  .and_then([&](const auto&& _) { return this->dal->write_node(unbalanced_node); });
              }

            return nullptr;
          });
      }

    /* left balance */
    if (unbalanced_node_index != pnode.children.size() - 1)
      {
        this->dal->get_node(pnode.children[unbalanced_node_index + 1])
          .and_then([&](auto&& right_node) -> tl::expected<std::nullptr_t, Error> {
            if (right_node.can_spare_an_element())
              {
                rotate_left(unbalanced_node, pnode, right_node, unbalanced_node_index);
                return this->dal->write_node(unbalanced_node)
                  .and_then([&](const auto&& _) { return this->dal->write_node(pnode); })
                  .and_then([&](const auto&& _) { return this->dal->write_node(right_node); });
              }

            return nullptr;
          });
      }

    /* The merge function merges a given node with its node to the right.
     * So by default, we merge an unbalanced node with its right sibling.
     * In the case where the unbalanced node is the leftmost,
     * we have to replace the merge parameters, so the unbalanced node right sibling,
     * will be merged into the unbalanced node. */
    if (unbalanced_node_index == 0)
      return this->dal->get_node(this->children[unbalanced_node_index + 1])
        .and_then([&](auto&& node) { return pnode.merge(node, unbalanced_node_index + 1); });

    return pnode.merge(unbalanced_node, unbalanced_node_index);
  }

  [[nodiscard]] auto Node::can_spare_an_element() const noexcept -> bool
  {
    if (this->dal->get_split_index(*this) == -1)
      return false;
    return true;
  }
} // namespace toocal::core::node
