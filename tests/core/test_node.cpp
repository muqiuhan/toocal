#include <doctest/doctest.h>
#include <numeric>
#include <string>
#include <vector>
#include "node.h"
#include "page.h"

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
          std::vector<page::Page_num>{},
        })
        .value();

    const auto node = types::Serializer<Node>::deserialize(buffer);

    CHECK_EQ(node->items.size(), 2);
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
