#include "data_access_layer.h"
#include "collection.h"
#include "utils.h"

using namespace toocal::core;
using namespace toocal::core::data_access_layer;
using namespace toocal::core::collection;
using namespace toocal::core::page;

int main(int argc, char ** argv)
{
  const auto data_size = 10000;
  const auto collection_name = std::string{"collection1"};

  auto dal = Data_access_layer{__FILE_NAME__".db"};

  auto collection = Collection{
    &dal, std::vector<uint8_t>{collection_name.begin(), collection_name.end()}, dal.meta.root};

  std::string keys[data_size], values[data_size];

  const auto generation_start_time = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < data_size; i++)
    {
      keys[i] = fmt::format("Key{}", i);
      values[i] = fmt::format("Value{}", i);
    }

  for (uint32_t i = 0; i < data_size; i++)
    {
      collection
        .put(
          std::vector<uint8_t>{keys[i].begin(), keys[i].end()},
          std::vector<uint8_t>{values[i].begin(), values[i].end()})
        .map_error([&](const auto && error) { return error.panic(); });
    }
  auto generation_end_time = std::chrono::high_resolution_clock::now();

  spdlog::info("querying {} data...", data_size);
  const auto finding_start_time = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < data_size; i++)
    {
      collection.find(std::vector<uint8_t>{keys[i].begin(), keys[i].end()})
        .map([&](const auto && item) {
          if (item == tl::nullopt)
            fatal("item is nullopt");

          auto       finding_end_time = std::chrono::high_resolution_clock::now();
          const auto finded_key = std::string{item.value().key.begin(), item.value().key.end()},
                     finded_value =
                       std::string{item.value().value.begin(), item.value().value.end()};

          if (keys[i] != finded_key)
            fatal(fmt::format("finded key is {}", finded_key));

          if (values[i] != finded_value)
            fatal(fmt::format("finded value is {}", finded_key));
        })
        .map_error([&](const auto && error) { return error.panic(); });
    }
  const auto finding_end_time = std::chrono::high_resolution_clock::now();
  spdlog::info(
    "querying completed, spend {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(finding_end_time - finding_start_time)
      .count());

  spdlog::info("removing db file...{}KB", utils::Filesystem::sizeof_file(dal.path));
  std::filesystem::remove(dal.path);

  spdlog::info("closing db...");
  dal.close();

  return 0;
}
