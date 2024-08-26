#ifndef TOOCAL_CORE_TYPES
#define TOOCAL_CORE_TYPES

#include "errors.hpp"
#include <fmt/core.h>
#include <tl/expected.hpp>

namespace toocal::core::types
{
  template <typename _Tp_serialize_type> class Serializer
  {
  public:
    [[nodiscard]] static auto serialize(const _Tp_serialize_type &self) noexcept
      -> tl::expected<std::vector<std::uint8_t>, errors::Error>
    {
      unimplemented();
    }

    [[nodiscard]] static auto deserialize(const std::vector<std::uint8_t> &buffer) noexcept
      -> tl::expected<_Tp_serialize_type, errors::Error>
    {
      unimplemented();
    }
  };
}; // namespace toocal::core::types

#endif /* TOOCAL_CORE_TYPES */