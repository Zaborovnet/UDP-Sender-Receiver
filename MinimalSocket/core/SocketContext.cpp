

#include <MinimalSocket/Error.h>
#include <MinimalSocket/core/SocketContext.h>

#include "../SocketFunctions.h"
#include "../SocketHandler.h"

namespace MinimalSocket {
Address RemoteAddressAware::getRemoteAddress() const {
  std::scoped_lock lock(remote_address_mtx);
  return remote_address;
}

RemoteAddressAware::RemoteAddressAware(const Address &address)
    : remote_address(address) {}
} // namespace MinimalSocket
