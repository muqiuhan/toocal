#include "data_access_layer.h"
#include "collection.h"
#include "utils.h"

using namespace toocal::core;
using namespace toocal::core::data_access_layer;
using namespace toocal::core::collection;
using namespace toocal::core::page;

int main(int argc, char ** argv)
{
  const auto collection_name = std::string{"collection1"};

  auto dal =
    Data_access_layer{"test_collection_put_big_data.db"};

  auto collection = Collection{
    &dal, std::vector<uint8_t>{collection_name.begin(), collection_name.end()}, dal.meta.root};

  spdlog::info("10k data are being generated and stored in {}", dal.path);
  const auto generation_start_time = std::chrono::high_resolution_clock::now();
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
  auto generation_end_time = std::chrono::high_resolution_clock::now();
  spdlog::info(
    "generation completed, spend {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(
      generation_end_time - generation_start_time)
      .count());

  spdlog::info("finding 10k data...");
  const auto finding_start_time = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 1; i <= 10000; i++)
    {
      const auto key = fmt::format("Key{}", i);
      const auto value = fmt::format("Value{}", i);

      collection.find(std::vector<uint8_t>{key.begin(), key.end()})
        .map([&](const auto && item) {
          if (item == tl::nullopt)
            fatal("item is nullopt");

          const auto finded_key = std::string{item.value().key.begin(), item.value().key.end()},
                     finded_value =
                       std::string{item.value().value.begin(), item.value().value.end()};
          auto finding_end_time = std::chrono::high_resolution_clock::now();

          if (key != finded_key)
            fatal(fmt::format("finded key is {}", finded_key));

          if (value != finded_value)
            fatal(fmt::format("finded value is {}", finded_key));
        })
        .map_error([&](const auto && error) { return error.panic(); });
    }
  const auto finding_end_time = std::chrono::high_resolution_clock::now();
  spdlog::info(
    "finding completed, spend {}ms",
    std::chrono::duration_cast<std::chrono::milliseconds>(finding_end_time - finding_start_time)
      .count());

  spdlog::info("removing db file...{}KB", utils::Filesystem::sizeof_file(dal.path));
  std::filesystem::remove(dal.path);

  spdlog::info("closing db...");
  dal.close();

  return 0;
}
