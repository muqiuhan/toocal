#include "data_access_layer.h"
#include "collection.h"

using namespace toocal::core;
using namespace toocal::core::data_access_layer;
using namespace toocal::core::collection;
using namespace toocal::core::page;

int main(int argc, char ** argv)
{
  const auto data_size = 10000;
  const auto collection_name = std::string{"collection1"};

  auto dal = Data_access_layer{__FILE_NAME__ ".db", builtin_options::BEST_PERFORMANCE};

  auto collection = Collection{
    &dal, std::vector<uint8_t>{collection_name.begin(), collection_name.end()}, dal.meta.root};

  std::string keys[data_size], values[data_size];

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

  spdlog::info("removing 10k data...");
  const auto removing_start_time = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < data_size; i++)
    {
      collection.remove(std::vector<uint8_t>{keys[i].begin(), keys[i].end()})
        .map_error([&](const auto && error) { return error.panic(); });
    }

  const auto removing_end_time = std::chrono::high_resolution_clock::now();
  spdlog::info(
    "removing completed, spend {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(removing_end_time - removing_end_time)
      .count());

  std::filesystem::remove(dal.path);
  dal.close();

  return 0;
}
