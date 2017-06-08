#ifndef ARG_TYPES_H
#define ARG_TYPES_H

#include <xmmintrin.h>

// Works only with variables, which size <= sizeof(size_t)
template <typename ... Args>
struct args_types;

template <>
struct args_types<>
{
  static const int INTEGER_CLASS = 0;
  static const int SSE_CLASS = 0;
  static const bool valid = true;
};

template <typename First, typename ... Args>
struct args_types<First, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS + 1;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS;
  static const bool valid = (sizeof(First) <= sizeof(size_t)) && args_types<Args ...>::valid;
};

template <typename ... Args>
struct args_types<float, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
  static const bool valid = args_types<Args ...>::valid;
};

template <typename ... Args>
struct args_types<double, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
  static const bool valid = args_types<Args ...>::valid;
};

template <typename ... Args>
struct args_types<__m64, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
  static const bool valid = args_types<Args ...>::valid;
};

#endif // ARG_TYPES_H




