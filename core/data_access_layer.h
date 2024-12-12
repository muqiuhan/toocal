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
  };

  namespace builtin_options
  {
    /** The default configuration, balanced mode, database file size and query,
     ** insert and other performance are moderate.
     ** The test results of 100k data:
     **   - generate and insert: 1.25s
     **   - query: 0.65s
     **   - file size: 472kb */
    inline static const auto BALANCE = Options{Page::DEFAULT_PAGE_SIZE, 0.5f, 0.5f};

    /** Optimize database file size, database file size is large, but query performance is low.
     ** The test results of 100k data:
     **   - generate and insert: 1.5s
     **   - query: 0.78s
     **   - file size: 324kb */
    inline static const auto BEST_FILE_SIZE = Options{Page::DEFAULT_PAGE_SIZE, 0.75f, 0.75f};

    /** Optimize query performance, database file size is small, but query performance is high.
     ** The test results of 100k data:
     **   - generate and insert: 0.49s
     **   - query: 0.21s
     **   - file size: 1816kb */
    inline static const auto BEST_PERFORMANCE = Options{Page::DEFAULT_PAGE_SIZE, 0.125f, 0.125f};
  } // namespace builtin_options

  class Data_access_layer
  {
  public:
    const std::string path;
    const Options     options;

    std::fstream file;
    Meta         meta;
    Freelist     freelist;

    explicit Data_access_layer(std::string path)
      : Data_access_layer(std::move(path), builtin_options::BALANCE)
    {}

    explicit Data_access_layer(std::string path, const Options options)
      : path(std::move(path)), options(options), meta({})
    {
      if (std::filesystem::exists(this->path))
        this->load_database().map_error([&](const auto&& error) { error.panic(); });
      else
        this->initialize_database().map_error([&](const auto&& error) { error.panic(); });
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
    [[nodiscard]] auto get_split_index(const Node& node) const noexcept -> uint32_t;

    [[nodiscard]] auto max_threshold() const noexcept -> float;
    [[nodiscard]] auto min_threshold() const noexcept -> float;
    [[nodiscard]] auto is_under_populated(const Node& node) const noexcept -> bool;
    [[nodiscard]] auto is_over_populated(const Node& node) const noexcept -> bool;

    /** Allocate an empty page. Different from directly constructing Page,
     ** this function will fill in a Page.data of option.page_size size. */
    [[nodiscard]] auto allocate_empty_page(page::Page_num page_num) const noexcept -> Page;

    /** Write a page. This function will check the operation results of all
     ** fstream functions during the writing process. If it fails, it will use
     ** std::strerror(errno) to construct an Error and return it. */
    [[nodiscard]] auto write_page(const Page& page) noexcept -> tl::expected<std::nullptr_t, Error>;

    /** Read a page. The error handling method is the same as write_page. */
    [[nodiscard]] auto read_page(page::Page_num page_num) noexcept -> tl::expected<Page, Error>;

    /** Use read_page to read freelist and return it. */
    [[nodiscard]] auto read_freelist() noexcept -> tl::expected<Freelist, Error>;

    /** Use write_page to read freelist and return it. */
    [[nodiscard]] auto write_freelist() noexcept -> tl::expected<std::nullptr_t, Error>;

    /** Use read_page to read meta and return it. */
    [[nodiscard]] auto read_meta() noexcept -> tl::expected<Meta, Error>;

    /** Use read_page to write meta and return it. */
    [[nodiscard]] auto write_meta(const Meta& meta) noexcept -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto
      new_node(std::deque<node::Item> items, std::deque<page::Page_num> children) noexcept -> Node;

    [[nodiscard]] auto write_node(Node& node) noexcept -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto write_node(const Node&& node) noexcept
      -> tl::expected<std::nullptr_t, Error>;

    [[nodiscard]] auto get_node(page::Page_num page_num) noexcept -> tl::expected<Node, Error>;

    auto delete_node(page::Page_num page_num) noexcept -> void;

  private:
    /** During the process of creating the Data access layer, if the database
     ** file does not exist in the target path, it is initialized. */
    auto initialize_database() noexcept -> tl::expected<std::nullptr_t, Error>;

    /** During the process of creating the Data access layer, If the database
     ** file exists at the target path, load it. */
    auto load_database() noexcept -> tl::expected<std::nullptr_t, Error>;
  }; // namespace toocal::core::data_access_layer
} // namespace toocal::core::data_access_layer

#endif /* TOOCAL_CORE_DATA_ACCESS_LAYER_H */
