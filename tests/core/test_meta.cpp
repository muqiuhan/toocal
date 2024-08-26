#include <doctest/doctest.h>
#include "meta.h"

namespace toocal::core::meta::tests
{
  TEST_CASE("serializer")
  {
    const auto buffer = types::Serializer<Meta>::serialize(Meta{2, 1}).value();
    const auto meta = types::Serializer<Meta>::deserialize(buffer);

    CHECK_EQ(meta->freelist_page, 1);
    CHECK_EQ(meta->root, 2);
  }
} // namespace toocal::core::meta::tests