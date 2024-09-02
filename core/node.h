#ifndef TOOCAL_CORE_NODE_H
#define TOOCAL_CORE_NODE_H

#include "data_access_layer.h"
#include "errors.hpp"
#include "page.h"
#include <cstdint>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

namespace toocal::core::node
{
  using data_access_layer::Data_access_layer;

  class Item
  {
  public:
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;

  public:
    [[nodiscard]] auto size() const noexcept -> uint32_t;
  };

  class Node
  {
  public:
    Data_access_layer          *dal;
    page::Page_num              page_num;
    std::vector<Item>           items;
    std::vector<page::Page_num> children;

  public:
    Node(std::vector<Item> items, std::vector<page::Page_num> children)
      : items(std::move(items)), children(std::move(children))
    {}

  public:
    [[nodiscard]] auto is_leaf() const noexcept -> bool;
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
                   /* fitem.key.size() + item.value.size() + offset */
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
       * offsets have a fixed size and are appended from the left. It's easier to
       * preserve the logical order (alphabetical in the case of b-tree) using the
       * metadata and performing pointer arithmetic. Using the data itself is
       * harder as it varies by size.
       *
       * Page structure is:
       * -----------------------------------------------------------------------
       * |  Page  | key-value /  child node    key-value 		    | key-value    |
       * | Header |   offset /	 pointer	  offset         .... | data .....   |
       * -----------------------------------------------------------------------
       */
      uint32_t left = 3, right = buffer.size() - 1;
      for (int i = 0; i < items_count; i++)
        {
          const auto item = self.items[i];

          /* Write the child page as a fixed size of 8 bytes */
          if (!is_leaf)
            {
              const auto span = std::span(buffer.begin() + left, buffer.end());
              endian::stream_writer<endian::little_endian>(
                span.data(), span.size())
                << self.children[i];

              left += sizeof(page::Page_num);
            }

          const auto key_size = static_cast<uint16_t>(item.key.size()),
                     value_size = static_cast<uint16_t>(item.value.size());

          uint16_t offset = right - key_size - value_size - sizeof(uint16_t);

          /* write offset */
          {
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
              .write(item.value.data(), value_size);

            right -= 1;
            buffer[right] = static_cast<uint8_t>(value_size);
          }

          {
            right -= key_size;
            const auto span = std::span(buffer.begin() + right, buffer.end());
            endian::stream_writer<endian::little_endian>(
              span.data(), span.size())
              .write(item.key.data(), key_size);

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