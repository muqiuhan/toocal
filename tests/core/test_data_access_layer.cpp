#include <doctest/doctest.h>
#include "data_access_layer.h"

namespace toocal::core::data_access_layer::tests
{
  TEST_CASE("initialize database")
  {
    auto dal = Data_access_layer{
      "test_data_access_layer_initialize_database.db"};

    CHECK_EQ(dal.freelist.max_page, 2);
    CHECK_EQ(dal.meta.freelist_page, 1);

    dal.close();
    std::filesystem::remove(dal.path);
  }

  TEST_CASE("load database")
  {
    /* initialize database */
    {
      auto dal = Data_access_layer{"test_data_access_layer_load_database.db"};

      CHECK_EQ(dal.freelist.max_page, 2);
      CHECK_EQ(dal.meta.freelist_page, 1);

      dal.write_meta(dal.meta).and_then(
        [&](const auto & _) { return dal.write_freelist(); }).map_error(
        [&](auto && error) {
          error.append("error when close data access layer");
          return error.panic();
        });

      dal.close();
    }

    /* load database */
    auto dal = Data_access_layer{"test_data_access_layer_load_database.db"};

    CHECK_EQ(dal.freelist.max_page, 2);
    CHECK_EQ(dal.meta.freelist_page, 1);

    dal.close();
    std::filesystem::remove(dal.path);
  }
} // namespace toocal::core::data_access_layer::tests
