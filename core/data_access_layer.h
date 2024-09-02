#ifndef TOOCAL_CORE_DATA_ACCESS_LAYER_H
#define TOOCAL_CORE_DATA_ACCESS_LAYER_H

#include "errors.hpp"
#include "freelist.h"
#include "meta.h"
#include "page.h"
#include "tl/expected.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <utility>

#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

namespace toocal::core::data_access_layer
{
  using errors::Error;
  using freelist::Freelist;
  using meta::Meta;
  using page::Page;

  class Options
  {
  public:
    const uint32_t page_size;
    const float    min_fill_percent;
    const float    max_fill_percent;

  public:
    inline static const auto DEFAULT_FILL_PERCENT = std::make_pair(0.5f, 0.95f);
  };

  class Data_access_layer
  {
  public:
    const std::string path;
    const Options     options;

    std::fstream file;
    Meta         meta;
    Freelist     freelist;

  public:
    Data_access_layer(
      const std::string path, const Options options = Data_access_layer::DEFAULT_OPTIONS)
      : path(std::move(path)), options(std::move(options))
    {}

    ~Data_access_layer() { this->close(); }

  private:
    /** Get the virtual memory page size of the current operating system. Currently, only
     ** the POSIX standard is supported.
     ** TODO: Windows support. */
    [[nodiscard]] static auto get_system_page_size() noexcept -> uint32_t;

    /** During the process of creating the Data access layer, if the database file does
     ** not exist in the target path, it is initialized. */
    [[nodiscard]] auto
      intialize_database() noexcept -> tl::expected<Data_access_layer, Error>;

    /** During the process of creating the Data access layer, If the database file exists
     ** at the target path, load it. */
    [[nodiscard]] auto load_database() noexcept -> tl::expected<Data_access_layer, Error>;

    /** Manually close the Data access layer. Note: This function will be automatically
     ** called after the scope of the Data access layer ends, and there is usually no need
     ** to call it manually. */
    auto close() noexcept -> void;

    [[nodiscard]] auto allocate_empty_page(page::Page_num page_num) const noexcept -> Page;

    [[nodiscard]] auto
      write_page(const Page & page) noexcept -> tl::expected<std::nullptr_t, Error>;
      
    [[nodiscard]] auto
      read_page(page::Page_num page_num) noexcept -> tl::expected<Page, Error>;

  private:
    inline static const auto DEFAULT_PAGE_SIZE =
      Data_access_layer::get_system_page_size();

    inline static const auto DEFAULT_OPTIONS = Options{
      Data_access_layer::get_system_page_size(),
      Options::DEFAULT_FILL_PERCENT.first,
      Options::DEFAULT_FILL_PERCENT.second};
  }; // namespace toocal::core::data_access_layer
} // namespace toocal::core::data_access_layer

#endif /* TOOCAL_CORE_DATA_ACCESS_LAYER_H */