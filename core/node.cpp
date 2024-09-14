#include "node.h"
#include "errors.hpp"
#include "page.h"
#include "data_access_layer.h"
#include <numeric>
#include <stdexcept>

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
        const auto &item = this->items.at(index);
        return item.key.size() + item.value.size() + sizeof(Page::page_num);
      }
    catch (const std::out_of_range &e)
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
      [&](const uint32_t size, const auto &item) {
        return size + item.size();
      });
  }

  [[nodiscard]] auto Node::is_last(
    uint32_t index, const Node &parent_node) const noexcept -> bool
  {
    return index == parent_node.items.size();
  }

  [[nodiscard]] auto Node::is_first(uint32_t index) const noexcept -> bool
  {
    return index == 0;
  }

  [[nodiscard]] auto Node::find_key_helper(
    const Node                 *node,
    const std::vector<uint8_t> &key,
    bool                        exact,
    std::vector<uint32_t>      &ancestors_indexes) const noexcept
    -> tl::expected<std::tuple<int, const Node *>, Error>
  {
    const auto &&[was_found, index] = this->find_key_in_node(key);
    if (was_found)
      return std::make_tuple(index, this);
    else if (this->is_leaf())
      if (exact)
	return std::make_tuple(-1, nullptr);
      else
	return std::make_tuple(index, this);
    else
      {
	ancestors_indexes.push_back(index);
	const auto next_child =
	  this->dal->get_node(this->children.at(index))
	    .map_error([](auto &&error) {
	      error.append("get_node error in find_key_helper");
	      return error;
	    });

	if (next_child.has_value())
	  return this->find_key_helper(
	    &next_child.value(), key, exact, ancestors_indexes);
	else
	  return tl::unexpected(next_child.error());
      }
  }

  [[nodiscard]] auto Node::find_key_in_node(const std::vector<uint8_t> &key)
    const noexcept -> std::tuple<bool, uint32_t>
  {
    for (uint32_t index = 0; auto existing_item : this->items)
      {
	const auto compare_result =
	  std::memcmp(existing_item.key.data(), key.data(), key.size());
	if (compare_result == 0)
	  return {true, index};
	else if (compare_result > 0)
	  return {false, index};
	else
	  ++index;
      }

    /* The key is bigger than the previous item, so it doesn't exist in the
     * node, but may exist in child nodes. */
    return {false, this->items.size()};
  }

  [[nodiscard]] auto
    Node::find_key(const std::vector<uint8_t> &key, bool exact) const noexcept
    -> tl::expected<std::tuple<int, const Node *, std::vector<uint32_t>>, Error>
  {
    auto ancestors_indexes = std::vector<uint32_t>{/* index of root */ 0};

    return this->find_key_helper(this, key, exact, ancestors_indexes)
      .map([&](const auto &&result) {
	const auto [index, node] = result;
	return std::make_tuple(index, node, ancestors_indexes);
      })
      .map_error([](auto &&error) {
	error.append("find_key_helper error in find_key");
	return error;
      });
  }
} // namespace toocal::core::node
