#ifndef TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP
#define TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP

/** \file The OpenCL SYCL pipe accessor<> detail behind the scene

    Ronan at Keryell point FR

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <cstddef>

#include "CL/sycl/access.hpp"
#include "CL/sycl/detail/debug.hpp"
#include "CL/sycl/pipe/detail/pipe.hpp"

namespace cl {
namespace sycl {

class handler;

namespace detail {

// Forward declaration of detail::accessor to declare the specialization
template <typename T,
          std::size_t Dimensions,
          access::mode Mode,
          access::target Target /* = access::global_buffer */>
struct accessor;
/** \addtogroup data Data access and storage in SYCL
    @{
*/

/** The accessor abstracts the way pipe data are accessed inside a
    kernel
*/
template <typename T,
          access::mode AccessMode>
struct accessor<T, 1, AccessMode, access::pipe> :
    public detail::debug<accessor<T, 1, AccessMode, access::pipe>> {
  /// The STL-like types
  using value_type = T;
  using reference = value_type&;
  using const_reference = const value_type&;

  /** The real pipe implementation behind the hood

      Since it is a reference instead of value member, it is a mutable
      state here, so that it can work with a [=] lambda capture
      without having to declare the whole lambda as mutable
   */
  detail::pipe<T> &implementation;

  /** Store the success status of last pipe operation

      It does exists even if the pipe accessor is not evaluated in a
      boolean context for, but a use-def analysis can optimise it out
      in that case and not use some storage

      Use a mutable state here so that it can work with a [=] lambda
      capture without having to declare the whole lambda as mutable
  */
  bool mutable ok = false;


  /** Construct a pipe accessor from an existing pipe
   */
  accessor(detail::pipe<T> &p, handler &command_group_handler) :
    implementation { p } {
    //    TRISYCL_DUMP_T("Create a kernel pipe accessor write = "
    //                 << is_write_access());
  }


  /** Return the maximum number of elements that can fit in the pipe
   */
  std::size_t capacity() const {
    return implementation.capacity();
  }

  /** Get the current number of elements in the pipe

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement (for
      example on FPGA).
   */
  std::size_t size() const {
    return implementation.size();
  }


  /** Test if the pipe is empty

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      write side (for example on FPGA).
   */
  bool empty() const {
    return implementation.empty();
  }


  /** Test if the pipe is full

      This is obviously a volatile value which is constrained by
      restricted relativity.

      Note that on some devices it may be costly to implement on the
      read side (for example on FPGA).
  */
  bool full() const {
    return implementation.full();
  }


  /** In a bool context, the accessor gives the success status of the
      last access

      \return true on success
  */
  operator bool() const {
    return ok;
  }


  /** Try to write a value to the pipe

      \param[in] value is what we want to write

      \return  this so we can apply a sequence of write for example
      (but do not do this on a non blocking pipe...)

      \todo provide a && version

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  const accessor &write(const T &value) const {
    ok = implementation.write(value);
    // Return a reference to *this so we can apply a sequence of write
    return *this;
  }


  /// Some syntactic sugar to use a << v instead of a.write(v)
  const accessor &operator<<(const T &value) const {
    // Return a reference to *this so we can apply a sequence of >>
    return write(value);
  }


  /** Try to read a value from the pipe

      \param[out] value is the reference to where to store what is
      read

      \return *this so we can apply a sequence of read for example
      (but do not do this on a non blocking pipe...)

      This function is const so it can work when the accessor is
      passed by copy in the [=] kernel lambda, which is not mutable by
      default
  */
  const accessor &read(T &value) const {
    ok = implementation.read(value);
    // Return a reference to *this so we can apply a sequence of read
    return *this;
  }


  /// Some syntactic sugar to use a >> v instead of a.read(v)
  const accessor &operator>>(T &value) const {
    // Return a reference to *this so we can apply a sequence of >>
    return read(value);
  }

};

/// @} End the data Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif // TRISYCL_SYCL_PIPE_DETAIL_ACCESSOR_HPP
