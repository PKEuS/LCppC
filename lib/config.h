#ifndef configH
#define configH

#ifdef _WIN32
#  ifdef CPPCHECKLIB_EXPORT
#    define CPPCHECKLIB __declspec(dllexport)
#  elif defined(CPPCHECKLIB_IMPORT)
#    define CPPCHECKLIB __declspec(dllimport)
#  else
#    define CPPCHECKLIB
#  endif
#else
#  define CPPCHECKLIB
#endif

#ifdef _WIN32
#  define NOMINMAX
#endif

// MS Visual C++ memory leak debug tracing
#if defined(_MSC_VER) && defined(_DEBUG)
#  define _CRTDBG_MAP_ALLOC
#  include <crtdbg.h>
#endif

// C++11 noreturn
#define NORETURN [[noreturn]]

#define REQUIRES(msg, ...) class=typename std::enable_if<__VA_ARGS__::value>::type

#include <string>
static const std::string emptyString;

#endif // configH
