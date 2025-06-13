// First, check if our build system wants to enable force-inlining
#ifdef ENABLE_FORCE_INLINING

  // Next, handle compiler specifics
  #if defined(_MSC_VER) // Microsoft Visual C++
    #define FORCE_INLINE __forceinline
  #elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
    #define FORCE_INLINE __attribute__((always_inline)) inline
  #else
    #define FORCE_INLINE inline // Fallback for unknown compilers
  #endif

#else
  // If the macro is not defined, FORCE_INLINE does nothing special
  #define FORCE_INLINE inline
#endif