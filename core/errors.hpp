#ifndef TOOCAL_CORE_ERRORS_HPP
#define TOOCAL_CORE_ERRORS_HPP

#include <exception>
#include <source_location>
#include <spdlog/spdlog.h>
#include <string>

namespace toocal::core::errors
{
  class Error
  {
  public:
    const std::string          message;
    const std::source_location location;

    auto append(std::string append_message) const noexcept -> void
    {
      spdlog::error("{} at ({}:{})", append_message, location.file_name(), location.line());
    }

    [[noreturn]] auto panic() const noexcept -> void
    {
      spdlog::error("{} at ({}:{})", message, location.file_name(), location.line());
      std::terminate();
    }
  };

#define _error(message)                                                                            \
  toocal::core::errors::Error { message, std::source_location::current() }

#define unimplemented()                                                                            \
  _error(                                                                                          \
    fmt::format("unimplemented function: {}", std::source_location::current().function_name()))    \
    .panic()

#define fatal(message) _error(fmt::format("fatal error: {}", message)).panic()

#define Err(message)   tl::make_unexpected(_error(message))
}; // namespace toocal::core::errors

#endif /* TOOCAL_CORE_ERRORS_HPP */
