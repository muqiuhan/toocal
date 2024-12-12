#ifndef TOOCAL_CORE_TYPES
#define TOOCAL_CORE_TYPES

#include "errors.hpp"
#include <tl/expected.hpp>
#include <deque>

namespace toocal::core::types
{
  using errors::Error;

  template <typename Tp_serialize_type> class Serializer
  {
  public:
    [[nodiscard]] static auto serialize(const Tp_serialize_type &) noexcept
      -> tl::expected<std::vector<uint8_t>, Error>
    {
      unimplemented();
    }

    [[nodiscard]] static auto deserialize(const std::vector<uint8_t> &) noexcept
      -> tl::expected<Tp_serialize_type, Error>
    {
      unimplemented();
    }
  };
} // namespace toocal::core::types

#endif /* TOOCAL_CORE_TYPES */