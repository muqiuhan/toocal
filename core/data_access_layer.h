#ifndef TOOCAL_CORE_DATA_ACCESS_LAYER_H
#define TOOCAL_CORE_DATA_ACCESS_LAYER_H

#include "errors.hpp"
#include "freelist.h"
#include "meta.h"
#include "node.h"
#include "page.h"
#include "tl/expected.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
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
  using node::Node;
  using page::Page;

  class Options
  {
  public:
    const uint32_t page_size;
    const float    min_fill_percent;
    const float    max_fill_percent;

    static constexpr auto DEFAULT_FILL_PERCENT = std::make_pair(0.5f, 0.95f);
  };

  class Data_access_layer
  {
  public:
    const std::string path;
    const Options     options;

    std::fstream file;
    Meta         meta;
    Freelist     freelist;

    explicit Data_access_layer(std::string path)
      : Data_access_layer(std::move(path), DEFAULT_OPTIONS)
    {}

    explicit Data_access_layer(std::string path, const Options options)
      : path(std::move(path)), options(options), meta({})
    {
      if (std::filesystem::exists(this->path))
        this->load_database().map_error(
          [&](const auto&& error) { error.panic(); });
      else
        this->intialize_database().map_error(
          [&](const auto&& error) { error.panic(); });
    }

    ~Data_access_layer() { this->close(); }

    /** Manually close the Data access layer. Note: This function will be
     ** automatically called after the scope of the Data access layer ends, and
     ** there is usually no need to call it manually. */
    auto close() noexcept -> void;

    /** get_split_index should be called when performing rebalance after an item
     ** is removed. It checks if a node can spare an element, and if it does
     ** then it returns the index when there the split should happen. Otherwise
     ** -1 is returned. */
    [[nodiscard]] auto
      get_split_index(const Node& node) const noexcept -> uint32_t;

    [[nodiscard]] auto max_threshold() const noexcept -> float;
    [[nodiscard]] auto min_threshold() const noexcept -> float;
    [[nodiscard]] auto
      is_under_populated(const Node& node) const noexcept -> bool;
    [[nodiscard]] auto
      is_over_populated(const Node& node) const noexcept -> bool;

    /** Allocate an empty page. Different from directly constructing Page,
     ** this function will fill in a Page.data of option.page_size size. */
    [[nodiscard]] auto
      allocate_empty_page(page::Page_num page_num) const noexcept -> Page;

    /** Write a page. This function will check the operation results of all
     ** fstream functions during the writing process. If it fails, it will use
     ** std::strerror(errno) to construct an Error and return it. */
    [[nodiscard]] auto write_page(const Page& page) noexcept
      -> tl::expected<std::nullptr_t, Error>;

    /** Read a page. The error handling method is the same as write_page. */
    [[nodiscard]] auto
      read_page(page::Page_num page_num) noexcept -> tl::expected<Page, Error>;

    /** Use read_page to read freelist and return it. */
    [[nodiscard]] auto read_freelist() noexcept -> tl::expected<Freelist, Error>;

    /** Use write_page to read freelist and return it. */
    [[nodiscard]] auto
      write_freelist() noexcept -> tl::expected<std::nullptr_t, Error>;

    /** Use read_page to read meta and return it. */
    [[nodiscard]] auto read_meta() noexcept -> tl::expected<Meta, Error>;

    /** Use read_page to write meta and return it. */
    [[nodiscard]] auto write_meta(const Meta& meta) noexcept
      -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto new_node(
      std::vector<node::Item>     items,
      std::vector<page::Page_num> children) noexcept -> Node;

    [[nodiscard]] auto
      write_node(Node& node) noexcept -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto write_node(const Node&& node) noexcept
      -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto
      get_node(page::Page_num page_num) noexcept -> tl::expected<Node, Error>;

    auto delete_node(page::Page_num page_num) noexcept -> void;

  private:
    /** Get the virtual memory page size of the current operating system.
     ** Currently, only the POSIX standard is supported.
     ** TODO: Windows support. */
    [[nodiscard]] static auto get_system_page_size() noexcept -> uint32_t;

    /** During the process of creating the Data access layer, if the database
     ** file does not exist in the target path, it is initialized. */
    auto intialize_database() noexcept -> tl::expected<std::nullptr_t, Error>;

    /** During the process of creating the Data access layer, If the database
     ** file exists at the target path, load it. */
    auto load_database() noexcept -> tl::expected<std::nullptr_t, Error>;

  public:
    inline static const auto DEFAULT_PAGE_SIZE =
      Data_access_layer::get_system_page_size();

    inline static const auto DEFAULT_OPTIONS = Options{
      Data_access_layer::DEFAULT_PAGE_SIZE,
      Options::DEFAULT_FILL_PERCENT.first,
      Options::DEFAULT_FILL_PERCENT.second};
  }; // namespace toocal::core::data_access_layer
} // namespace toocal::core::data_access_layer

#endif /* TOOCAL_CORE_DATA_ACCESS_LAYER_H */
