#include <doctest/doctest.h>
#include "freelist.h"
#include "page.h"

namespace toocal::core::freelist::tests
{
  TEST_CASE("serializer")
  {
    const auto buffer =
      types::Serializer<Freelist>::serialize(Freelist{1, std::vector<page::Page_num>(10)})
        .value();

    const auto freelist = types::Serializer<Freelist>::deserialize(buffer);

    CHECK_EQ(freelist->max_page, 1);
    CHECK_EQ(freelist->released_pages.size(), 10);
  }

  TEST_CASE("get_next_page")
  {
    auto freelist = Freelist{};

    CHECK_EQ(freelist.get_next_page(), 1);
    CHECK_EQ(freelist.max_page, 1);
  }
} // namespace toocal::core::freelist::tests