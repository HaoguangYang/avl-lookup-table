/**
 * @file lookup_table.hpp
 * @author Haoguang Yang (yang1510@purdue.edu)
 * @version 0.1
 * @date 2023-11-26
 *
 * @copyright Copyright (c) 2023 Haoguang Yang
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 *
 */

#ifndef ND_LOOKUP_TABLE__LOOKUP_TABLE_HPP_
#define ND_LOOKUP_TABLE__LOOKUP_TABLE_HPP_

#include <stddef.h>

#include <array>
#include <utility>
#include <vector>

#include "avl_tree.hpp"

namespace LUT {

template <typename KeyT, typename ValueT>
int compareKeys(const std::pair<KeyT, ValueT>& lhs, const std::pair<KeyT, ValueT>& rhs) {
  if (lhs.first == rhs.first) return 0;
  if (lhs.first > rhs.first) return 1;
  return -1;
}

template <typename KeyT, typename ValueT, size_t dim = 1UL>
class LookupTable
    : protected AVL_Tree<std::pair<KeyT, LookupTable<KeyT, ValueT, dim - 1>*>,
                         decltype(*(compareKeys<KeyT, LookupTable<KeyT, ValueT, dim - 1>*>))> {
 private:
  typedef LookupTable<KeyT, ValueT, dim - 1>* SubLUT_PtrT;

 public:
  typedef std::pair<KeyT, SubLUT_PtrT> LUT_DataT;
  typedef decltype(*(compareKeys<KeyT, SubLUT_PtrT>)) LUT_CompFunc;

  LookupTable() : AVL_Tree<LUT_DataT, LUT_CompFunc> { *(compareKeys<KeyT, SubLUT_PtrT>) }
  {}

  void insert(const LUT_DataT& data) {
    this->root_ = AVL_Tree<LUT_DataT, LUT_CompFunc>::insert(this->root_, data, this->comp_);
  }

  void insert(const KeyT& key, const SubLUT_PtrT& val) { insert({key, val}); }

  void insert(const std::vector<LUT_DataT>& data) {
    for (const auto& d : data) insert(d);
  }

  void remove(const LUT_DataT& data) {
    this->root_ = AVL_Tree<LUT_DataT, LUT_CompFunc>::remove(this->root_, data, this->comp_);
  }

  void remove(const KeyT& key, const SubLUT_PtrT& val) { remove({key, val}); }

  void remove(const std::vector<LUT_DataT>& data) {
    for (const auto& d : data) remove(d);
  }

  ValueT lookup(const std::array<KeyT, dim>& input) {
    std::pair<const LUT_DataT*, const LUT_DataT*> closest_pair =
        static_cast<AVL_Tree<LUT_DataT, LUT_CompFunc>*>(this)->lookup({input[0], nullptr});
    return interp(closest_pair, input).second;
  }

  std::vector<ValueT> lookup(const std::vector<std::array<KeyT, dim>>& input) {
    std::vector<ValueT> ret;
    ret.reserve(input.size());
    for (const auto& i : input) ret.emplace_back(lookup(i));
    return ret;
  }

 protected:
  std::pair<KeyT, ValueT> interp_impl(const LUT_DataT& lower, const LUT_DataT& upper,
                                      const std::array<KeyT, dim>& inp) {
    std::pair<KeyT, ValueT> ret{inp[0], ValueT{}};
    if (ret.first == lower.first) {
      ret.second = lower.second->lookup({inp.cbegin() + 1, inp.cend()});
    } else if (ret.first == upper.first) {
      ret.second = upper.second->lookup({inp.cbegin() + 1, inp.cend()});
    } else {
      // FIXME: merge the results from two sub-lookup tables, using multi-linear interpolation.
      double t = (ret.first - lower.first) / (upper.first - lower.first);
      ValueT lower_val = lower.second->lookup({inp.cbegin() + 1, inp.cend()});
      ValueT upper_val = upper.second->lookup({inp.cbegin() + 1, inp.cend()});
      ret.second = lower_val + t * (upper_val - lower_val);
    }
    return ret;
  }

  std::pair<KeyT, ValueT> interp(const std::pair<const LUT_DataT*, const LUT_DataT*>& ref,
                                 const std::array<KeyT, dim>& input) {
    std::pair<KeyT, ValueT> ret{};
    if (ref.first == nullptr) {
      if (ref.second == nullptr) return ret;
      ret = *(ref.second);
    } else if (ref.second == nullptr) {
      ret = *(ref.first);
    } else {
      ret = interp_impl(*(ref.first), *(ref.second), input);
    }
    return ret;
  }
};

// Specialization on a 1-D LUT
template <typename KeyT, typename ValueT>
class LookupTable<KeyT, ValueT, 1UL>
    : protected AVL_Tree<std::pair<KeyT, ValueT>, decltype(*(compareKeys<KeyT, ValueT>))> {
 public:
  typedef std::pair<KeyT, ValueT> LUT_DataT;
  typedef decltype(*(compareKeys<KeyT, ValueT>)) LUT_CompFunc;

  LookupTable() : AVL_Tree<LUT_DataT, LUT_CompFunc> { *(compareKeys<KeyT, ValueT>) }
  {}

  void insert(const LUT_DataT& data) {
    this->root_ = AVL_Tree<LUT_DataT, LUT_CompFunc>::insert(this->root_, data, this->comp_);
  }

  void insert(const KeyT& key, const ValueT& val) { insert({key, val}); }

  void insert(const std::vector<LUT_DataT>& data) {
    for (const auto& d : data) insert(d);
  }

  void remove(const LUT_DataT& data) {
    this->root_ = AVL_Tree<LUT_DataT, LUT_CompFunc>::remove(this->root_, data, this->comp_);
  }

  void remove(const KeyT& key, const ValueT& val) { remove({key, val}); }

  void remove(const std::vector<LUT_DataT>& data) {
    for (const auto& d : data) remove(d);
  }

  ValueT lookup(const KeyT& input) {
    LUT_DataT lookup_inp{input, ValueT{}};
    std::pair<const LUT_DataT*, const LUT_DataT*> closest_pair =
        static_cast<AVL_Tree<LUT_DataT, LUT_CompFunc>*>(this)->lookup(lookup_inp);
    return interp(closest_pair, lookup_inp).second;
  }

  std::vector<ValueT> lookup(const std::vector<KeyT>& input) {
    std::vector<ValueT> ret;
    ret.reserve(input.size());
    for (const auto& i : input) ret.emplace_back(lookup(i));
    return ret;
  }

 protected:
  LUT_DataT interp_impl(const LUT_DataT& lower, const LUT_DataT& upper, const LUT_DataT& inp) {
    LUT_DataT ret = inp;
    if (ret.first == lower.first) {
      ret.second = lower.second;
    } else if (ret.first == upper.first) {
      ret.second = upper.second;
    } else {
      double t = (ret.first - lower.first) / (upper.first - lower.first);
      ret.second = lower.second + t * (upper.second - lower.second);
    }
    return ret;
  }

  LUT_DataT interp(const std::pair<const LUT_DataT*, const LUT_DataT*>& ref,
                   const LUT_DataT& input) {
    LUT_DataT ret{};
    if (ref.first == nullptr) {
      if (ref.second == nullptr) return ret;
      ret = *(ref.second);
    } else if (ref.second == nullptr) {
      ret = *(ref.first);
    } else {
      ret = interp_impl(*(ref.first), *(ref.second), input);
    }
    return ret;
  }
};

}  // namespace LUT

#endif  // ND_LOOKUP_TABLE__LOOKUP_TABLE_HPP_
