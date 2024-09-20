#include <doctest/doctest.h>
#include "data_access_layer.h"

namespace toocal::core::collection::tests
{
  using data_access_layer::Data_access_layer;

  TEST_CASE("put")
  {
    auto dal = Data_access_layer{
      "./db.db", {Data_access_layer::DEFAULT_PAGE_SIZE, 0.0125, 0.025}};

    dal.close();
    std::filesystem::remove(dal.path);
  }
}; // namespace toocal::core::collection::tests