#include "node.h"
#include "errors.hpp"
#include "page.h"
#include "data_access_layer.h"
#include <cstdarg>
#include <numeric>
#include <span>
#include <stdexcept>
#include "utils.h"

namespace toocal::core::node
{
  [[nodiscard]] auto Node::is_leaf() const noexcept -> bool
  {
    return this->children.empty();
  }

  [[nodiscard]] auto Item::size() const noexcept -> uint32_t
  {
    return this->key.size() + this->value.size();
  }

  [[nodiscard]] auto Node::item_size(uint32_t index) const noexcept -> uint32_t
  {
    try
      {
        const auto & [key, value] = this->items.at(index);
        return key.size() + value.size() + sizeof(Page::page_num);
      }
    catch (const std::out_of_range & e)
      {
        fatal(fmt::format(
          "cannot get the item size of {} because out of range", index));
      }
  }

  [[nodiscard]] auto Node::size() const noexcept -> uint32_t
  {
    return std::accumulate(
      items.begin(),
      items.end(),
      static_cast<uint32_t>(Node::HEADER_SIZE + sizeof(page::Page_num)),
      [&](const uint32_t size, const auto & item) {
        return size + item.size();
      });
  }

  [[nodiscard]] auto Node::is_last(
    const uint32_t index, const Node & parent_node) const noexcept -> bool
  {
    return index == parent_node.items.size();
  }

  [[nodiscard]] auto Node::is_first(const uint32_t index) const noexcept -> bool
  {
    return index == 0;
  }

  [[nodiscard]] auto Node::find_key_helper(
    const Node &                 node,
    const std::vector<uint8_t> & key,
    const bool                   exact,
    std::vector<uint32_t> &      ancestors_indexes) noexcept
    -> tl::expected<std::tuple<int, tl::optional<Node>>, Error>
  {
    const auto && [was_found, index] = node.find_key_in_node(key);
    if (was_found)
      return std::make_tuple(index, node);

    if (node.is_leaf())
      {
        if (exact)
          return std::make_tuple(-1, tl::nullopt);
        return std::make_tuple(index, node);
      }

    ancestors_indexes.push_back(index);
    const auto next_child =
      node.dal->get_node(node.children.at(index)).map_error([](auto && error) {
        error.append("get_node error in find_key_helper");
        return error;
      });

    if (next_child.has_value())
      return find_key_helper(next_child.value(), key, exact, ancestors_indexes);
    return tl::unexpected(next_child.error());
  }

  [[nodiscard]] auto Node::find_key_in_node(const std::vector<uint8_t> & key)
    const noexcept -> std::tuple<bool, uint32_t>
  {
    for (uint32_t index = 0; const auto & [existing_item_key, _] : this->items)
      {
        const auto compare_result =
          utils::Safecmp::bytescmp(existing_item_key, key);
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

  [[nodiscard]] auto Node::find_key(
    const std::vector<uint8_t> & key, const bool exact) const noexcept
    -> tl::
      expected<std::tuple<int, tl::optional<Node>, std::vector<uint32_t>>, Error>
  {
    auto ancestors_indexes = std::vector<uint32_t>{/* index of root */ 0};

    return find_key_helper(*this, key, exact, ancestors_indexes)
      .map([&](const auto && result) {
        const auto [index, node] = result;
        return std::make_tuple(index, node, ancestors_indexes);
      });
  }

  auto Node::add_item(
    const Item & item, const uint32_t insertion_index) noexcept -> uint32_t
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
    Data_access_layer * dal,
    Node &              node_to_split,
    const uint32_t      split_index,
    Node *              new_node) noexcept -> void
  {
    dal
      ->write_node(dal->new_node(
        std::vector<Item>{
          node_to_split.items.begin() + split_index + 1,
          node_to_split.items.end()},
        std::vector<page::Page_num>{}))
      .map([&](const auto && node) {
        new_node = std::move(node);
        node_to_split.items = std::vector<Item>{
          node_to_split.items.begin(),
          node_to_split.items.begin() + split_index};

        return nullptr;
      })
      .map_error([&](auto && error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

  static auto _split_when_node_is_not_leaf(
    Data_access_layer * dal,
    Node &              node_to_split,
    const uint32_t      split_index,
    Node *              new_node) noexcept -> void
  {
    dal
      ->write_node(dal->new_node(
        std::vector<Item>{
          node_to_split.items.begin() + split_index + 1,
          node_to_split.items.end()},
        std::vector<page::Page_num>{
          node_to_split.children.begin() + split_index + 1,
          node_to_split.children.end()}))
      .map([&](const auto && node) {
        new_node = std::move(node);
        node_to_split.items = std::vector<Item>{
          node_to_split.items.begin(),
          node_to_split.items.begin() + split_index};

        node_to_split.children = std::vector<page::Page_num>{
          node_to_split.children.begin(),
          node_to_split.children.begin() + split_index + 1};

        return nullptr;
      })
      .map_error([&](auto && error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

  auto Node::split(
    Node & node_to_split, const uint32_t node_to_split_index) noexcept -> void
  {
    /* the first index where min amount of bytes to populate a page is archived.
     * then add 1 so it will be split one index after. */
    const auto split_index = node_to_split.dal->get_split_index(node_to_split);
    const auto middle_item = node_to_split.items.at(split_index);

    Node * new_node = nullptr;
    if (node_to_split.is_leaf())
      _split_when_node_is_leaf(this->dal, node_to_split, split_index, new_node);
    else
      _split_when_node_is_not_leaf(
        this->dal, node_to_split, split_index, new_node);

    this->add_item(middle_item, node_to_split_index);

    /* if middle of list, then move items forward. */
    if (this->children.size() == node_to_split_index + 1)
      this->children.push_back(new_node->page_num);
    else
      this->children.insert(
        this->children.begin() + node_to_split_index + 1, new_node->page_num);

    this->dal->write_node(*this)
      .and_then(
        [&](const auto && _) { return this->dal->write_node(node_to_split); })
      .map_error([](auto && error) {
        error.append("write_node error in Node::split");
        return error.panic();
      });
  }

} // namespace toocal::core::node
