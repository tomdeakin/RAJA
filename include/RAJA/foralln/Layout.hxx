//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
// 
// LLNL-CODE-689114
// 
// All rights reserved.
// 
// This file is part of RAJA. 
// 
// For additional details, please also read raja/README-license.txt.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the disclaimer below.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//


#ifndef RAJA_LAYOUT_HXX__
#define RAJA_LAYOUT_HXX__

#include "RAJA/LegacyCompatibility.hxx"
#include "RAJA/foralln/Permutations.hxx"
#include "RAJA/IndexValue.hxx"
#include <iostream>
#include <limits>

namespace RAJA {

/******************************************************************
 *  Permutation tags
 ******************************************************************/
 


template<typename Range, typename Perm, typename IdxLin, typename ... IDs>
struct Layout_impl ;
template<size_t ... RangeInts,size_t ... PermInts, typename IdxLin, typename ... IDs>
struct Layout_impl<VarOps::index_sequence<RangeInts...>, VarOps::index_sequence<PermInts...>, IdxLin, IDs...> {

  typedef VarOps::index_sequence<PermInts...> Permutation;
  typedef IdxLin IndexLinear;
  typedef std::tuple<IDs...> IDtuple;
  typedef VarOps::make_index_sequence<sizeof...(IDs)> IndexRange;

  static constexpr size_t n_dims = sizeof...(IDs);
  static constexpr size_t limit = std::numeric_limits<Index_type>::max();

  // const char *index_types[sizeof...(IDs)];

  Index_type perms[sizeof...(IDs)];
  Index_type sizes[sizeof...(IDs)];
  Index_type strides[sizeof...(IDs)];
  Index_type mods[sizeof...(IDs)];

  // TODO: this should be constexpr in c++14 mode
  template<typename ... Types>
  RAJA_INLINE RAJA_HOST_DEVICE  Layout_impl(Types... ns):
    sizes{convertIndex<Index_type>(ns)...}
    // index_types{typeid(IDs).name()...}
  {
        VarOps::assign_args(perms, IndexRange{}, PermInts...);
        Index_type swizzled_sizes[] = {sizes[PermInts]...};
        Index_type folded_strides[n_dims];
        for (int i=0; i < n_dims; i++) {
            folded_strides[i] = 1;
            for (int j=0; j < i; j++) {
                folded_strides[j] *= swizzled_sizes[i];
            }
        }
        assign(strides, folded_strides, Permutation{}, IndexRange{});

        Index_type lmods[n_dims];
        for (int i=1; i < n_dims; i++) {
            lmods[i] = folded_strides[i-1];
        }
        lmods[0] = limit;

        assign(mods,lmods, Permutation{}, IndexRange{});
    }

  RAJA_INLINE RAJA_HOST_DEVICE constexpr IdxLin operator()(IDs ... indices) const {
      return convertIndex<IdxLin>(
              VarOps::sum<Index_type>((convertIndex<Index_type>(indices) * strides[RangeInts])...));
  }

  RAJA_INLINE RAJA_HOST_DEVICE void toIndices(IdxLin linear_index, IDs &...indices) const {
      VarOps::assign_args(linear_index, IndexRange{}, indices...);
  }
};

template<size_t ... RangeInts, size_t ... PermInts, typename IdxLin, typename ... IDs>
constexpr size_t Layout_impl<VarOps::index_sequence<RangeInts...>, VarOps::index_sequence<PermInts...>, IdxLin, IDs...>::n_dims;
template<size_t ... RangeInts, size_t ... PermInts, typename IdxLin, typename ... IDs>
constexpr size_t Layout_impl<VarOps::index_sequence<RangeInts...>, VarOps::index_sequence<PermInts...>, IdxLin, IDs...>::limit;

template<typename IdxLin, typename Permutation, typename ... IDs>
using Layout = Layout_impl<VarOps::make_index_sequence<sizeof...(IDs)>, Permutation, IdxLin, IDs...>;


} // namespace RAJA

template<typename...Args>
std::ostream &operator<<(std::ostream &os, RAJA::Layout_impl<Args...> const &m) {
    os << "permutation:" << m.perms << std::endl;
    os << "sizes:" << m.sizes << std::endl;
    os << "mods:" << m.mods << std::endl;
    os << "strides:" << m.strides << std::endl;
    return os;
}

#endif
  
