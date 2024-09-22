#include "data_access_layer.h"
#include "collection.h"

using namespace toocal::core::data_access_layer;
using namespace toocal::core::collection;
using namespace toocal::core::page;

int main(int argc, char ** argv)
{
  const auto collection_name = std::string{"collection1"};
  const auto key1 = std::string{"Key1"};
  const auto value1 = std::string{"Value1"};

  auto dal = Data_access_layer{
    "test_collection_put_big_data.db",
    {Page::DEFAULT_PAGE_SIZE, 0.0125, 0.025}};

  auto collection = Collection{
    &dal,
    std::vector<uint8_t>{collection_name.begin(), collection_name.end()},
    dal.meta.root};

  for (uint32_t i = 1; i <= 10000; i++)
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
      if (item == tl::nullopt)
        fatal("item is nullopt");

      const auto finded_key1 =
                   std::string{
                     item.value().key.begin(), item.value().key.end()},
                 finded_value1 = std::string{
                   item.value().value.begin(), item.value().value.end()};
      if (key1 != finded_key1)
        fatal(fmt::format("finded key1 is {}", finded_key1));

      if (value1 != finded_value1)
        fatal(fmt::format("finded value1 is {}", finded_key1));
    })
    .map_error([&](const auto && error) { return error.panic(); });

  std::filesystem::remove(dal.path);
  dal.close();
}
