#include "node.h"

namespace toocal::core::node
{
  [[nodiscard]] auto Node::is_leaf() const noexcept -> bool
  {
    return this->children.empty();
  }

  [[nodiscard]] auto Item::size() const noexcept -> uint32_t
  {
    return this->key.size() + this->value.size();
  }
} // namespace toocal::core::node