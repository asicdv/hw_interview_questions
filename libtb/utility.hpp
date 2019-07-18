//========================================================================== //
// Copyright (c) 2019, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

#ifndef __UTILITY_HPP__
#define __UTILITY_HPP__

namespace tb {

template<typename T>
constexpr std::size_t bits() { return 8 * sizeof(T); }

template<typename T>
constexpr T mask(std::size_t n) { return (static_cast<T>(1) << n) - 1; }

template<typename T>
constexpr bool bit(const T & t, std::size_t i) { return (t >> i) & 0x1; }

template<typename T>
constexpr void set_bit(T & t, std::size_t i) { t |= (1 << i); }

template<typename T>
std::size_t pop_count(T t) {
  std::size_t cnt{0};
  while (t) {
    if (t & 0x1)
      cnt++;
    t >>= 1;
  }
  return cnt;
}

template<typename T>
T clear_range(T t, std::size_t lsb, std::size_t msb) {
  return t & (~mask<T>(msb - lsb + 1) << lsb);
}
  
template<typename T, typename U>
T set_range_bits(T v, U f, std::size_t lsb, std::size_t bits) {
  T out{clear_range(v, lsb, lsb + bits - 1)};
  out |= (static_cast<T>(f) & mask<T>(bits)) << lsb;
  return out;
}

} // namespace

#endif
