// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
// Separate executable: replaces global operator new/delete to count allocations.
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <cstdlib>
#include <new>
#include "support/TestSupport.hpp"
static std::atomic<bool> gCount{false}; static std::atomic<long> gAllocs{0};
void* operator new(std::size_t n) { if (gCount) ++gAllocs; if (void* p = std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void* operator new[](std::size_t n) { if (gCount) ++gAllocs; if (void* p = std::malloc(n ? n : 1)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
using namespace refractor; using namespace rt;

// T-022 [operational/performance] No heap allocation in reset/setParameter/process. Enforces: SC-7, D-014.
TEST_CASE("T-022 real-time path does not allocate", "[operational][T-022]") {
  const double fs = 48000; Engine e; e.prepare(fs, 1024); neutral(e); e.reset();
  const Vec x = noise(size_t(fs), 3u); Vec y(x.size());
  gAllocs = 0; gCount = true;
  e.reset();
  for (size_t p = 0; p + 128 <= x.size(); p += 128) {
    const float v = float((p / 128) % 100) / 100.0f;
    for (int id = 0; id < int(ParamId::Count); ++id) e.setParameter(ParamId(id), v);
    const float* i[1] = { x.data() + p }; float* o[1] = { y.data() + p }; e.process(i, o, 1, 1, 128);
  }
  gCount = false;
  CHECK(gAllocs.load() == 0);
}
