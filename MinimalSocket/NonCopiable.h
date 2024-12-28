

#pragma once

namespace MinimalSocket {
class NonCopiable {
public:
  NonCopiable(const NonCopiable &) = delete;
  NonCopiable &operator=(const NonCopiable &) = delete;

protected:
  NonCopiable() = default;
};
} // namespace MinimalSocket
