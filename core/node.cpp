#include "node.h"
#include "errors.hpp"
#include "page.h"
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
} // namespace toocal::core::node