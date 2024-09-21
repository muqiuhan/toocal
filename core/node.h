#ifndef TOOCAL_CORE_NODE_H
#define TOOCAL_CORE_NODE_H

#include "errors.hpp"
#include "page.h"
#include <cstdint>
#include <numeric>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include "tl/expected.hpp"
#include "tl/optional.hpp"
#include "types.hpp"
#include <endian/stream_reader.hpp>
#include <endian/stream_writer.hpp>
#include <endian/little_endian.hpp>

namespace toocal::core::data_access_layer
{
  class Data_access_layer;
}

namespace toocal::core::node
{
  using data_access_layer::Data_access_layer;
  using errors::Error;
  using page::Page;
  using types::Serializer;

  class Item
  {
  public:
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;

    [[nodiscard]] auto size() const noexcept -> uint32_t;
  };

  class Node
  {
  public:
    Data_access_layer          *dal{};
    page::Page_num              page_num{};
    std::vector<Item>           items;
    std::vector<page::Page_num> children;

    Node() = default;

    Node(std::vector<Item> items, std::vector<page::Page_num> children)
      : items(std::move(items)), children(std::move(children))
    {}

    Node(
      Data_access_layer          *dal,
      const page::Page_num        page_num,
      std::vector<Item>           items,
      std::vector<page::Page_num> children)
      : dal(dal)
      , page_num(page_num)
      , items(std::move(items))
      , children(std::move(children))
    {}

    /** Determine whether the current node is a leaf node. */
    [[nodiscard]] auto is_leaf() const noexcept -> bool;

    [[nodiscard]] auto
      is_last(uint32_t index, const Node &parent_node) const noexcept -> bool;

    [[nodiscard]] auto is_first(uint32_t index) const noexcept -> bool;

    /** Returns the size of a key-value-childNode triplet at a given index. If
     ** the node is a leaf, then the size of a key-value pair is returned. It's
     ** assumed i <= len(n.items) */
    [[nodiscard]] auto item_size(uint32_t index) const noexcept -> uint32_t;

    /** Returns the node's size in bytes */
    [[nodiscard]] auto size() const noexcept -> uint32_t;

    /** Searches for a key inside the tree. Once the key is found, the
     ** parent node and the correct index are returned so the key itself can be
     ** accessed in the following way parent[index]. A list of the node
     ** ancestors (not including the node itself) is also returned. If the key
     ** is not found, we have 2 options. If exact is true, it means we expect
     ** find_key to find the key, so a falsey answer. If exact is false, then
     ** find_key is used to locate where a new key should be inserted so the
     ** position is returned. */
    [[nodiscard]] auto
      find_key(const std::vector<uint8_t> &key, bool exact) const noexcept
      -> tl::expected<
        std::tuple<int, tl::optional<Node>, std::vector<uint32_t>>,
        Error>;

    auto
      add_item(const Item &item, uint32_t insertion_index) noexcept -> uint32_t;

    /* Checks if the node size is bigger than the size of a page. */
    [[nodiscard]] auto is_over_populated() const noexcept -> bool;

    /* Checks if the node size is smaller than the size of a page. */
    [[nodiscard]] auto is_under_populated() const noexcept -> bool;

    /** split rebalances the tree after adding. After insertion the modified
     ** node has to be checked to make sure it didn't exceed the maximum
     *number
     ** of elements. If it did, then it has to be split and rebalanced. The
     ** mapation is depicted in the graph below. If it's not a leaf node,
     ** then the children has to be moved as well as shown. This may leave the
     ** parent unbalanced by having too many items so rebalancing has to be
     ** checked for all the ancestors. The split is performed in a for loop to
     ** support splitting a node more than once. (Though in practice used only
     ** once):
     ** 	       n                                  n
     **                3                                 3,6
     **	            /    \       ------>       /          |           \
     **	          a    modifiedNode            a       modifiedNode  newNode
     **         1,2      4,5,6,7,8            1,2          4,5         7,8 */
    auto
      split(Node &node_to_split, uint32_t node_to_split_index) noexcept -> void;

  private:
    /** find_key_in_node iterates all the items and finds the key. If the key is
     ** found, then the item is returned. If the key isn't found then return the
     ** index where it should have been (the first index that key is greater
     ** than it's previous). */
    [[nodiscard]] auto find_key_in_node(const std::vector<uint8_t> &key)
      const noexcept -> std::tuple<bool, uint32_t>;

    [[nodiscard]] static auto find_key_helper(
      const Node                 &node,
      const std::vector<uint8_t> &key,
      bool                        exact,
      std::vector<uint32_t>      &ancestors_indexes) noexcept
      -> tl::expected<std::tuple<int, tl::optional<Node>>, Error>;

  public:
    static constexpr uint32_t HEADER_SIZE = 3;
  };
} // namespace toocal::core::node

namespace toocal::core::types
{
  using errors::Error;
  using node::Node;

  template <> class Serializer<Node>
  {
  public:
    [[nodiscard]] static auto serialize(const Node &self) noexcept
      -> tl::expected<std::vector<std::uint8_t>, Error>
    {
      const auto is_leaf = self.is_leaf();
      const auto items_count = static_cast<uint16_t>(self.items.size());

      auto buffer = std::vector<uint8_t>(
        sizeof(static_cast<uint8_t>(is_leaf)) + sizeof(items_count)
        + std::accumulate(
          self.items.begin(),
          self.items.end(),
          0,
          [&](const auto &items_size, const auto &item) {
            return items_size +
                   /* item.key.size() + item.value.size() + offset */
                   (sizeof(uint8_t) * 3) +
                   /* offset */ sizeof(uint16_t) + item.size();
          }));

      /* serialize header (is_leaf, items_count) */
      {
        endian::stream_writer<endian::little_endian>(
          buffer.data(), buffer.size())
          << static_cast<uint8_t>(is_leaf ? 1 : 0) << items_count;
      }

      /* We use slotted pages for storing data in the page. It means the actual
       * keys and values (the cells) are appended to right of the page whereas
       * offsets have a fixed size and are appended from the left. It's easier
       * to preserve the logical order (alphabetical in the case of b-tree)
       * using the metadata and performing pointer arithmetic. Using the data
       * itself is harder as it varies by size.
       *
       * Page structure is:
       * --------------------------------------------------------------------
       * |  Page  | key-value /  child node    key-value        | key-value |
       * | Header |   offset /	 pointer	      offset     .... | data .....|
       * --------------------------------------------------------------------
       */
      uint32_t left = 3, right = buffer.size() - 1;
      for (int i = 0; i < items_count; i++)
        {
          const auto [key, value] = self.items[i];

          /* Write the child page as a fixed size of 8 bytes */
          if (!is_leaf)
            {
              const auto span = std::span(buffer.begin() + left, buffer.end());
              endian::stream_writer<endian::little_endian>(
                span.data(), span.size())
                << self.children[i];

              left += sizeof(page::Page_num);
            }

          const auto key_size = static_cast<uint16_t>(key.size()),
                     value_size = static_cast<uint16_t>(value.size());

          /* write offset */
          {
            uint16_t offset = right - key_size - value_size - sizeof(uint16_t);
            const auto span = std::span(buffer.begin() + left, buffer.end());
            endian::stream_writer<endian::little_endian>(
              span.data(), span.size())
              << offset;

            left += sizeof(uint16_t);
          }

          {
            right -= value_size;
            const auto span = std::span(buffer.begin() + right, buffer.end());
            endian::stream_writer<endian::little_endian>(
              span.data(), span.size())
              .write(value.data(), value_size);

            right -= 1;
            buffer[right] = static_cast<uint8_t>(value_size);
          }

          {
            right -= key_size;
            const auto span = std::span(buffer.begin() + right, buffer.end());
            endian::stream_writer<endian::little_endian>(
              span.data(), span.size())
              .write(key.data(), key_size);

            right -= 1;
            buffer[right] = static_cast<uint8_t>(key_size);
          }
        }

      /* Write the last child node; */
      if (!is_leaf)
        {
          const auto span = std::span(buffer.begin() + left, buffer.end());
          endian::stream_writer<endian::little_endian>(span.data(), span.size())
            << self.children.back();
        }

      return buffer;
    }

    [[nodiscard]] static auto
      deserialize(const std::vector<std::uint8_t> &buffer) noexcept
      -> tl::expected<Node, Error>
    {
      uint8_t  is_leaf;
      uint16_t items_count;

      {
        endian::stream_reader<endian::little_endian>(
          buffer.data(), buffer.size())
          >> is_leaf >> items_count;
      }

      auto     children = std::vector<page::Page_num>{};
      auto     items = std::vector<node::Item>{items_count};
      uint32_t left = 3;
      for (uint32_t i = 0; i < items_count; i++)
        {
          if (0 == is_leaf)
            {
              const auto span = std::span(buffer.begin() + left, buffer.end());
              page::Page_num page_num;

              endian::stream_reader<endian::little_endian>(
                span.data(), span.size())
                >> page_num;
              children.push_back(page_num);
              left += sizeof(page::Page_num);
            }

          uint16_t offset;
          {
            const auto span = std::span(buffer.begin() + left, buffer.end());
            endian::stream_reader<endian::little_endian>(
              span.data(), span.size())
              >> offset;
            left += sizeof(uint16_t);
          }

          const uint16_t key_size = buffer[offset];
          offset += 1;

          items[i].key = std::vector<uint8_t>(key_size);
          {
            const auto span = std::span(
              buffer.begin() + offset, buffer.begin() + offset + key_size);
            endian::stream_reader<endian::little_endian>(
              span.data(), span.size())
              .read(items[i].key.data(), key_size);

            offset += key_size;
          }

          const uint16_t value_size = buffer[offset];
          offset += 1;

          items[i].value = std::vector<uint8_t>(value_size);
          {
            const auto span = std::span(
              buffer.begin() + offset, buffer.begin() + offset + value_size);
            endian::stream_reader<endian::little_endian>(
              span.data(), span.size())
              .read(items[i].value.data(), value_size);

            offset += key_size;
          }
        }

      if (0 == is_leaf)
        {
          const auto     span = std::span(buffer.begin() + left, buffer.end());
          page::Page_num page_num;

          endian::stream_reader<endian::little_endian>(span.data(), span.size())
            >> page_num;
          children.push_back(page_num);
        }

      return Node{items, children};
    }
  };
} // namespace toocal::core::types

#endif /* TOOCAL_CORE_NODE_H */
