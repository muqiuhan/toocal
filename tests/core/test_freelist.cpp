#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tl/expected.hpp>
#include <vector>

#include "freelist.h"
#include "errors.hpp"
#include "page.h"
#include "spdlog/fmt/bundled/core.h"

namespace toocal::core::freelist::tests
{
  static auto test_serializer(std::nullptr_t) noexcept
    -> tl::expected<std::nullptr_t, core::errors::Error>
  {
    const auto buffer =
      types::Serializer<Freelist>::serialize(Freelist{1, std::vector<page::Page_num>(10)})
        .value();
    const auto freelist = types::Serializer<Freelist>::deserialize(buffer);

    if (freelist->max_page != 1)
      return tl::unexpected(__error(
        fmt::format("freelist->max_page == {}, excepted {}", freelist->max_page, 1)));

    if (freelist->released_pages.size() != 10)
      return tl::unexpected(__error(fmt::format(
        "freelist->released_pages.size() == {}, excepted {}",
        freelist->released_pages.size(),
        10)));

    return nullptr;
  }
} // namespace toocal::core::freelist::tests

auto main(int argc, char *argv[]) -> int
{
  using namespace toocal::core::freelist::tests;

  test_serializer(nullptr).or_else([](const auto &err) { err.panic(); });

  return 0;
}