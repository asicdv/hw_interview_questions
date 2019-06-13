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

#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <random>
#include <limits>
#include <random>
#include <algorithm>
#include <unordered_set>

namespace tb {

struct Random {

  // Uniform integral random number generator.
  //
  template <typename T>
  struct UniformRandomInterval {
    using value_type = T;
    
    UniformRandomInterval(T hi = std::numeric_limits<T>::max(),
                          T lo = std::numeric_limits<T>::min())
        : hi_(hi), lo_(lo) {
      dst_ = std::uniform_int_distribution<T>(lo_, hi_);
    }

    T lo() const { return lo_; }
    T hi() const { return hi_; }
    T operator()() { return dst_(Random::mt_); }

   private:
    std::uniform_int_distribution<T> dst_;
    T lo_, hi_;
  };

  // Filter functor that sources a sequence of values from the RND
  // class according to the filter 'FN' class.
  //
  template<typename RND, typename FN>
  struct Filter {
    using value_type = typename RND::value_type;
    
    Filter(RND & rnd, FN & fn) : rnd_(rnd), fn_(fn) {}
    value_type operator()() {
      value_type v;
      do { v = rnd_(); } while (!fn_(v));
      return v;
    }
   private:
    FN & fn_;
    RND & rnd_;
  };

  // Stateful filter predicate whereby values which have been seen
  // prior are rejected.
  //
  template<typename T>
  struct IsUniquePredicate {
    void reset() { prior_.clear(); }
    bool operator()(const T & t) {
      const bool ret{prior_.count(t) == 0};
      if (ret)
        prior_.insert(t);
      return ret;
    }
   private:
    std::unordered_set<T> prior_;
  };

  // Bag class; used to randomly pick from a set of values.
  //
  template<typename T>
  struct Bag {
    explicit Bag() {};
    void add(const T & t, std::size_t weight = 1, bool do_finalize = true) {
      while (weight--)
        v_.push_back(t);
      if (do_finalize)
        finalize();
    }
    template<typename FwdIt>
    void add(FwdIt begin, FwdIt end) {
      for (FwdIt it = begin; it != end; ++it)
        add(*it);
      finalize();
    }
    void clear() { v_.clear(); }
    void finalize() { r_ = UniformRandomInterval<std::size_t>{v_.size() - 1}; }
    T operator()() { return v_[r_()]; }
   private:
    UniformRandomInterval<std::size_t> r_;
    std::vector<T> v_;
  };

  // Function to shuffle some sequence of values pointed by two Random
  // Access Iterators.
  //
  template<typename RndIt>
  static void shuffle(RndIt begin, RndIt end) {
    std::shuffle(begin, end, Random::mt_);
  }

  static void set_seed(std::size_t seed) {
    Random::mt_.seed(seed);
  }

 private:
  static std::mt19937 mt_;
};

} // namespace tb

#endif
