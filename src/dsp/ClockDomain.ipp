#pragma once
#include "dsp/Errors.hpp"
template <class Fn> float refractor::ClockDomain::tick(float, Fn&&) { throw NotImplemented("ClockDomain::tick"); }
