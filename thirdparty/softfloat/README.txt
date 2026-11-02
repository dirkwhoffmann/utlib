Berkeley SoftFloat Release 3e (John R. Hauser)
https://github.com/ucb-bar/berkeley-softfloat-3  (see COPYING.txt for the license)

Only the parts of SoftFloat that utlib needs are included: the 80-bit extended
format (extF80) with its conversions to and from 32-bit and 64-bit integers
and the 32-bit and 64-bit IEEE formats, plus the required primitives. The
"8086" specialization (x87 semantics for NaNs and invalid conversions) is used.

UTLIB NOTE: This is a derivative work. Compared to the original distribution:

  - Platform.h is a utlib-specific file (it replaces the build script options).
  - THREAD_LOCAL in softfloat.h is defined for C, C++ and MSVC.
  - utl_env.h / utl_env.c were added. They give C++ code access to the
    thread-local state of the library.
  - Everything else is unmodified.

Use the class utl::Float80 (include/utl/math/Float80.h) instead of calling these
functions directly.
