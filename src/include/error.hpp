/**
 * Copyright (C) 2023 Muqiu Han <muqiu-han@outlook.com?
 *
 * This program can be distributed under the terms of the GNU GPLv3.
 * See the file COPYING.
 *
 * Since the point of this filesystem is to learn FUSE and its
 * datastructures, I want to see *everything* that happens related to
 * its data structures.  This file contains macros and functions to
 * accomplish this. */

#ifndef MOS_ERROR_H
#define MOS_ERROR_H

#include "log.hpp"
#include <cerrno>
#include <cstdint>
#include <exception>
#include <format>
#include <map>
#include <string>

namespace mos::error
{
  enum class Code
  {
    Undefined,
    System_call,
  };

  inline const std::map<Code, const char *> CODE_TO_STRING = {
    { Code::Undefined, "Unknown" },
    { Code::System_call, "System call" },
  };

  class Err
  {
   public:
    Err() { std::terminate(); }

    Err(const Code code,
        std::string custom,
        const uint32_t line,
        const char * file,
        const char * function)
    {
      const char * err_type = CODE_TO_STRING.at(code);
      const std::string custom_msg = [&]() {
        if (custom.empty())
          return std::string("");
        else
          return (": " + custom);
      }();

      LOG_ERROR << std::format(
        "{} Error with ERRNO({}) -> {}{} in function `{}` at <{}:{}>",
        err_type,
        errno,
        std::strerror(errno),
        custom_msg.c_str(),
        function,
        file,
        line);
    }
  };

} // namespace mos::error

#define ERR(CODE) Err(mos::error::Err(CODE, "", __LINE__, __FILE__, __FUNCTION__))
#define ERR_MSG(CODE, MSG)                                                               \
  Err(mos::error::Err(CODE, MSG, __LINE__, __FILE__, __FUNCTION__))

#endif /* sinbuger_ERROR_H */
