#pragma once

#if !defined(WARN_RESERVED_IDENTIFIER_OFF)
#if defined(__clang__)
#define WARN_RESERVED_IDENTIFIER_OFF \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wreserved-identifier\"")
#else
#define WARN_RESERVED_IDENTIFIER_OFF
#endif  // __clang__
#endif  // WARN_RESERVED_IDENTIFIER_OFF

#if !defined(WARN_RESERVED_IDENTIFIER_ON)
#if defined(__clang__)
#define WARN_RESERVED_IDENTIFIER_ON _Pragma("clang diagnostic pop")
#else
#define WARN_RESERVED_IDENTIFIER_ON
#endif  // __clang__
#endif  // WARN_RESERVED_IDENTIFIER_ON

#if !defined(WARN_C23_EXTENSIONS_OFF)
#if defined(__clang__)
#define WARN_C23_EXTENSIONS_OFF \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wc23-extensions\"")
#else
#define WARN_C23_EXTENSIONS_OFF
#endif  // __clang__
#endif  // WARN_C23_EXTENSIONS_OFF

#if !defined(WARN_C23_EXTENSIONS_ON)
#if defined(__clang__)
#define WARN_C23_EXTENSIONS_ON _Pragma("clang diagnostic pop")
#else
#define WARN_C23_EXTENSIONS_ON
#endif  // __clang__
#endif  // WARN_C23_EXTENSIONS_ON

#if !defined(WARN_DISABLED_MACRO_EXPANSION_OFF)
#if defined(__clang__)
#define WARN_DISABLED_MACRO_EXPANSION_OFF \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdisabled-macro-expansion\"")
#else
#define WARN_DISABLED_MACRO_EXPANSION_OFF
#endif  // __clang__
#endif  // WARN_DISABLED_MACRO_EXPANSION_OFF

#if !defined(WARN_DISABLED_MACRO_EXPANSION_ON)
#if defined(__clang__)
#define WARN_DISABLED_MACRO_EXPANSION_ON _Pragma("clang diagnostic pop")
#else
#define WARN_DISABLED_MACRO_EXPANSION_ON
#endif  // __clang__
#endif  // WARN_DISABLED_MACRO_EXPANSION_ON

#if !defined(WARN_UNUSED_MACROS_OFF)
#if defined(__clang__)
#define WARN_UNUSED_MACROS_OFF \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wunused-macros\"")
#else
#define WARN_UNUSED_MACROS_OFF
#endif  // __clang__
#endif  // WARN_UNUSED_MACROS_OFF

#if !defined(WARN_UNUSED_MACROS_ON)
#if defined(__clang__)
#define WARN_UNUSED_MACROS_ON _Pragma("clang diagnostic pop")
#else
#define WARN_UNUSED_MACROS_ON
#endif  // __clang__
#endif  // WARN_UNUSED_MACROS_ON
