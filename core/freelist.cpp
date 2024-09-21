#include "./freelist.h"

namespace toocal::core::freelist
{
  [[nodiscard]] auto Freelist::get_next_page() noexcept -> page::Page_num
  {
    // Take the last element and remove it from the list.
    if (!this->released_pages.empty())
      {
        const auto back = released_pages.back();
        released_pages.pop_back();
        return back;
      }

    return ++this->max_page;
  }

  auto Freelist::release_page(const page::Page_num page) noexcept -> void
  {
    this->released_pages.push_back(page);
  }
} // namespace toocal::core::freelist