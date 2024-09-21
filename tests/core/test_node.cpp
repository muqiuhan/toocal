#include <cstddef>
#include <doctest/doctest.h>
#include <numeric>
#include <string>
#include <vector>
#include "data_access_layer.h"
#include "errors.hpp"
#include "node.h"
#include "page.h"
#include "tl/optional.hpp"

namespace toocal::core::node::tests
{

  TEST_CASE("serializer")
  {
    const auto items = std::vector<std::pair<std::string, std::string>>{
      {"key1", "value1"},
      {"key2", "value2"},
    };

    const auto buffer =
      Serializer<Node>::serialize(
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

    const auto node = Serializer<Node>::deserialize(buffer);

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

  TEST_CASE("find key")
  {
    const auto key1 = std::string{"Key1"};
    const auto value1 = std::string{"value1"};

    auto dal = Data_access_layer{"../../../../tests/dbfiles/test_node_find.db"};
    dal.get_node(dal.meta.root)
      .and_then([&](auto &&node) {
        node.dal = &dal;
        return node.find_key(
          std::vector<uint8_t>{key1.begin(), key1.end()}, false);
      })
      .map([&](const auto &&result) {
        const auto [index, containing_node, _] = result;

        CHECK_NE(containing_node, tl::nullopt);

        const auto &item = containing_node.value().items.at(index);
        const auto  item_key = std::string{item.key.begin(), item.key.end()};
        const auto  item_value =
          std::string{item.value.begin(), item.value.end()};

        CHECK_EQ(item_key, key1);
        CHECK_EQ(item_value, value1);
      })
      .map_error([&](const auto &&error) { return error.panic(); });
    dal.close();
  }
} // namespace toocal::core::node::tests
