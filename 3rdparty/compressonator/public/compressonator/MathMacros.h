#ifndef COMPRESSONATOR_MATH_MACROS_H
#define COMPRESSONATOR_MATH_MACROS_H

#pragma warning(push)
#pragma warning(disable : 4018)

template <typename primNumType, typename secNumType>
inline decltype(auto) min(const primNumType& a, const secNumType& b)
{
  return (a < b ? a : b);
}

template <typename primNumType, typename secNumType>
inline decltype(auto) max(const primNumType& a, const secNumType& b)
{
  return (a > b ? a : b);
}
#pragma warning(pop)

#endif
