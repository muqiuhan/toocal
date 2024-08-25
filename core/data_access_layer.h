#ifndef TOOCAL_CORE_DATA_ACCESS_LAYER_H
#define TOOCAL_CORE_DATA_ACCESS_LAYER_H

#include <cstdint>
#include <tuple>

namespace toocal::core::data_access_layer
{
  class Options
  {
  public:
    const uint32_t page_size;
    const float min_fill_percent;
    const float max_fill_percent;

  public:
    inline static const auto DEFAULT_FILL_PERCENT = std::make_tuple(0.5, 0, 95);
  };

  class Data_access_layer
  {
    
  };
} // namespace toocal::core::data_access_layer

#endif /* TOOCAL_CORE_DATA_ACCESS_LAYER_H */