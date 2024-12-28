

#include <MinimalSocket/Error.h>

#include "MinimalSocket/SocketHandler.h"

namespace MinimalSocket {
namespace {
int getLastErrorCode() {
  int res =
#ifdef _WIN32
      WSAGetLastError();
#else
      static_cast<int>(errno);
#endif
  return res;
}
} // namespace

ErrorCodeHolder::ErrorCodeHolder() : errorCode{getLastErrorCode()} {}

SocketError::SocketError(const std::string &what)
    : ErrorCodeHolder{}, Error(what, " , error code: ", getErrorCode()) {}
} // namespace MinimalSocket
