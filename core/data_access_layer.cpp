#include "data_access_layer.h"
#include "errors.hpp"
#include "page.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include "tl/expected.hpp"
#include <cstdint>
#include <exception>
#include <vector>

namespace toocal::core::data_access_layer
{
  [[nodiscard]] auto
    Data_access_layer::get_system_page_size() noexcept -> uint32_t
  {
#ifdef __unix__
    return getpagesize();
#endif

// Not tested yet
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<uint32_t>(sysInfo.dwPageSize);
#endif
  }

  auto Data_access_layer::intialize_database() noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    this->freelist = Freelist{};
    this->meta.freelist_page = this->freelist.get_next_page();

    return this->write_freelist()
      .transform([&](const auto &&_) {
        /* init root */
        unimplemented();
        return nullptr;
      })
      .and_then([&](const auto &&_) { return this->write_meta(this->meta); });
  }

  auto Data_access_layer::load_database() noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    return this->read_meta()
      .and_then([&](const auto &&meta) {
        this->meta = meta;
        return this->read_freelist();
      })
      .transform([&](const auto &&freelist) {
        this->freelist = freelist;
        return nullptr;
      });
  }

  auto Data_access_layer::close() noexcept -> void { this->file.close(); }

  [[nodiscard]] auto Data_access_layer::allocate_empty_page(
    page::Page_num page_num) const noexcept -> Page
  {
    return Page{page_num, std::vector<uint8_t>(this->options.page_size)};
  }

  [[nodiscard]] auto Data_access_layer::write_page(const Page &page) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    if (this->file.seekp(page.page_num * Data_access_layer::DEFAULT_PAGE_SIZE)
          .fail())
      return Err(std::strerror(errno));

    if (this->file
          .write(
            reinterpret_cast<const char *>(page.data.data()), page.data.size())
          .fail())
      return Err(std::strerror(errno));

    return nullptr;
  }

  [[nodiscard]] auto Data_access_layer::read_page(
    page::Page_num page_num) noexcept -> tl::expected<Page, Error>
  {
    auto page = this->allocate_empty_page(page_num);
    if (this->file.seekg(page_num * Data_access_layer::DEFAULT_PAGE_SIZE).fail())
      return Err(std::strerror(errno));

    if (this->file
          .read(reinterpret_cast<char *>(page.data.data()), page.data.size())
          .fail())
      return Err(std::strerror(errno));

    return page;
  }

  [[nodiscard]] auto
    Data_access_layer::read_freelist() noexcept -> tl::expected<Freelist, Error>
  {
    return this->read_page(this->meta.freelist_page)
      .transform([&](const auto &&page) {
        return types::Serializer<Freelist>::deserialize(page.data)
          .transform_error([&](const auto &&error) { return error.panic(); })
          .value();
      });
  }

  [[nodiscard]] auto Data_access_layer::write_freelist() noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    auto page = this->allocate_empty_page(this->meta.freelist_page);
    return types::Serializer<Freelist>::serialize(this->freelist)
      .transform([&](const auto &&data) {
        page.data = data;
        return this->write_page(page)
          .transform_error([&](const auto &&error) { return error.panic(); })
          .value();
      });
  }

  [[nodiscard]] auto
    Data_access_layer::read_meta() noexcept -> tl::expected<Meta, Error>
  {
    return this->read_page(Meta::PAGE_NUM).transform([&](const auto &&page) {
      return types::Serializer<Meta>::deserialize(page.data)
        .transform_error([&](const auto &&error) { return error.panic(); })
        .value();
    });
  }

  [[nodiscard]] auto Data_access_layer::write_meta(const Meta &meta) noexcept
    -> tl::expected<std::nullptr_t, Error>
  {
    auto page = this->allocate_empty_page(Meta::PAGE_NUM);
    return types::Serializer<Meta>::serialize(meta).transform(
      [&](const auto &&data) {
        page.data = data;
        return this->write_page(page)
          .transform_error([&](const auto &&error) { return error.panic(); })
          .value();
      });
  }

} // namespace toocal::core::data_access_layer
