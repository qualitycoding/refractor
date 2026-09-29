#pragma once
#include <stdexcept>
namespace refractor {
/// Thrown by every stub. The implementer replaces each stub body; no NotImplemented may remain at S-014.
struct NotImplemented : std::logic_error { using std::logic_error::logic_error; };
}
