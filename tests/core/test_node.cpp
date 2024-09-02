#include <algorithm>
#include <doctest/doctest.h>
#include <numeric>
#include <vector>
#include "node.h"
#include "page.h"
#include "spdlog/fmt/bundled/core.h"

namespace toocal::core::node::tests
{

  TEST_CASE("serializer")
  {
    const auto items = std::vector<std::pair<std::string, std::string>>{
      {"key1", "value1"},
      {"key2", "value2"},
    };

    const auto buffer =
      types::Serializer<Node>::serialize(
        Node{
          std::accumulate(
            items.begin(),
            items.end(),
            std::vector<Item>{},
            [&](auto items, const auto &item) {
              items.push_back(Item{
                std::vector<uint8_t>(item.first.begin(), item.first.end()),
                std::vector<uint8_t>(item.second.begin(), item.second.end())});
              return items;
            }),
          std::vector<page::Page_num>{1, 2, 3, 4, 5},
        })
        .value();

    std::ranges::for_each(
      buffer, [&](const auto byte) { fmt::print("{} ", byte); });

    const auto node = types::Serializer<Node>::deserialize(buffer);

    CHECK_EQ(node->children.size(), 5);
    CHECK_EQ(node->children[0], 0);
    CHECK_EQ(node->children[4], 5);
    CHECK_EQ(
      std::string(node->items[0].key.begin(), node->items[0].key.end()),
      "key1");
    CHECK_EQ(
      std::string(node->items[0].value.begin(), node->items[0].value.end()),
      "value1");
    CHECK_EQ(
      std::string(node->items[1].key.begin(), node->items[1].key.end()),
      "key2");
    CHECK_EQ(
      std::string(node->items[1].value.begin(), node->items[1].value.end()),
      "value2");
  };
} // namespace toocal::core::node::tests