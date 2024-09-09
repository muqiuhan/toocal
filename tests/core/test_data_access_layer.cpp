#include <doctest/doctest.h>
#include "data_access_layer.h"

namespace toocal::core::data_access_layer::tests
{
  TEST_CASE("initialize database")
  {
    auto dal = Data_access_layer{"db.db"};
    CHECK_EQ(dal.freelist.max_page, 1);
    CHECK_EQ(dal.meta.freelist_page, 1);
    std::filesystem::remove(dal.path);
    dal.close();
  }

  TEST_CASE("load database")
  {
    /* initialize database */
    {
      auto dal = Data_access_layer{"db.db"};
      dal.close();
    }

    /* load database */
    auto dal = Data_access_layer{"db.db"};

    CHECK_EQ(dal.freelist.max_page, 1);
    CHECK_EQ(dal.meta.freelist_page, 1);

    dal.close();
    std::filesystem::remove(dal.path);
  }
} // namespace toocal::core::data_access_layer::tests