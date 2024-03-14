
#include "rpc_error.h"

struct ErrorCategory : public std::error_category {
  const char *name() const noexcept override { return "RpcError"; }

  std::string message(int ev) const override {
    switch (static_cast<rpc::Error>(ev)) {
      //   case FlightsErrc::NonexistentLocations:
      //     return "nonexistent airport name in request";
    default:
      return "(RpcError unrecognized error)";
    }
  }

} theErrCategory;

std::error_code make_error_code(rpc::Error e) {
  return {static_cast<int>(e), theErrCategory};
}
