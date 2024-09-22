#include <doctest/doctest.h>
#include <string>
#include "collection.h"
#include "data_access_layer.h"
#include "tl/optional.hpp"

namespace toocal::core::collection::tests
{
  using data_access_layer::Data_access_layer;

  TEST_CASE("put")
  {
    const auto collection_name = std::string{"collection1"};
    const auto key1 = std::string{"Key1"};
    const auto value1 = std::string{"Value1"};

    auto dal = Data_access_layer{
      "test_collection_put.db", {page::Page::DEFAULT_PAGE_SIZE, 0.0125, 0.025}};

    auto collection = Collection{
      &dal,
      std::vector<uint8_t>{collection_name.begin(), collection_name.end()},
      dal.meta.root};
    
    for (uint32_t i = 1; i <= 6; i++)
      {
        const auto key = fmt::format("Key{}", i);
        const auto value = fmt::format("Value{}", i);

        collection
          .put(
            std::vector<uint8_t>{key.begin(), key.end()},
            std::vector<uint8_t>{value.begin(), value.end()})
          .map_error([&](const auto && error) { return error.panic(); });
      }

    collection.find(std::vector<uint8_t>{key1.begin(), key1.end()})
      .map([&](const auto && item) {
        CHECK_NE(item, tl::nullopt);
        CHECK_EQ(
          key1, std::string{item.value().key.begin(), item.value().key.end()});
        CHECK_EQ(
          value1,
          std::string{item.value().value.begin(), item.value().value.end()});
      })
      .map_error([&](const auto && error) { return error.panic(); });

    dal.close();
    std::filesystem::remove(dal.path);
  }

  TEST_CASE("put big data")
  {
    const auto collection_name = std::string{"collection1"};
    const auto key1 = std::string{"Key1"};
    const auto value1 = std::string{"Value1"};

    auto dal = Data_access_layer{
      "test_collection_put_big_data.db", {page::Page::DEFAULT_PAGE_SIZE, 0.0125, 0.025}};

    auto collection = Collection{
      &dal,
      std::vector<uint8_t>{collection_name.begin(), collection_name.end()},
      dal.meta.root};

    for (uint32_t i = 1; i <= 100; i++)
      {
        const auto key = fmt::format("Key{}", i);
        const auto value = fmt::format("Value{}", i);

        collection
          .put(
            std::vector<uint8_t>{key.begin(), key.end()},
            std::vector<uint8_t>{value.begin(), value.end()})
          .map_error([&](const auto && error) { return error.panic(); });
      }

    collection.find(std::vector<uint8_t>{key1.begin(), key1.end()})
      .map([&](const auto && item) {
        CHECK_NE(item, tl::nullopt);
        CHECK_EQ(
          key1, std::string{item.value().key.begin(), item.value().key.end()});
        CHECK_EQ(
          value1,
          std::string{item.value().value.begin(), item.value().value.end()});
      })
      .map_error([&](const auto && error) { return error.panic(); });

    dal.close();
    std::filesystem::remove(dal.path);
  }
}; // namespace toocal::core::collection::tests