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

  public:
    [[noreturn]] auto panic() const noexcept -> void
    {
      spdlog::error(
        "{} at ({}:{})", message, location.file_name(), location.line());
      std::terminate();
    }
  };

#define __error(message)                                                       \
  toocal::core::errors::Error { message, std::source_location::current() }

#define unimplemented()                                                        \
  __error(fmt::format(                                                         \
            "unimplemented function: {}",                                      \
            std::source_location::current().function_name()))                  \
    .panic()

#define fatal(message) __error(fmt::format("fatal error: {}", message)).panic()

#define Err(message)   tl::make_unexpected(__error(message))
}; // namespace toocal::core::errors

#endif /* TOOCAL_CORE_ERRORS_HPP */
