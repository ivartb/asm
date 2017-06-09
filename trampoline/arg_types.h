#ifndef ARG_TYPES_H
#define ARG_TYPES_H

#include <xmmintrin.h>

template <typename ... Args>
struct args_types;

template <>
struct args_types<>
{
  static const int INTEGER_CLASS = 0;
  static const int SSE_CLASS = 0;
};

template <typename First, typename ... Args>
struct args_types<First, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS + 1;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS;
};

template <typename ... Args>
struct args_types<float, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
};

template <typename ... Args>
struct args_types<double, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
};

template <typename ... Args>
struct args_types<__m64, Args ...>
{
  static const int INTEGER_CLASS = args_types<Args ...>::INTEGER_CLASS;
  static const int SSE_CLASS = args_types<Args ...>::SSE_CLASS + 1;
};

#endif // ARG_TYPES_H




