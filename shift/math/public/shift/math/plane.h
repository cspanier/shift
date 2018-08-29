#ifndef SHIFT_MATH_PLANE_H
#define SHIFT_MATH_PLANE_H

#include "shift/math/vector.h"
#include "shift/math/matrix.h"

namespace shift::math
{
///
template <std::size_t N, typename T>
class plane
{
public:
  /// Default constructor.
  plane();

  /// Constructor.
  plane(T a, T b, T c, T d);

  /// Constructor.
  plane(const vector<N, T>& normal, T distance);

  /// Constructor.
  plane(const vector<N + 1, T>& v);

  /// Copy constructor.
  plane(const plane& other);

  /// Constructor.
  /// @remarks
  ///    This constructor is only defined for planes in T^3.
  plane(const vector3<T>& a, const vector3<T>& b, const vector3<T>& c);

  /// Assignement operator.
  plane& operator=(const plane& other);

  /// Negation operator.
  plane operator-();

public:
  ///
  const vector<N, T>& normal() const
  {
    return _normal;
  }

  ///
  void normal(const vector<N, T>& new_normal)
  {
    _normal = new_normal;
  }

  ///
  T distance() const
  {
    return _distance;
  }

  ///
  void distance(const T& new_distance)
  {
    _distance = new_distance;
  }

public:
  /// Negate plane.
  plane& negate();

  /// Renormalizes this plane.
  /// @return
  ///    Returns a reference to this object.
  plane<N, T>& normalize();

private:
  vector<N, T> _normal;
  T _distance;
};

template <std::size_t N, typename T>
plane<N, T>::plane() = default;

template <std::size_t N, typename T>
plane<N, T>::plane(T a, T b, T c, T d)
: _normal(vector<N, T>(a, b, c)), _distance(d)
{
}

template <std::size_t N, typename T>
plane<N, T>::plane(const vector<N, T>& normal, T distance)
: _normal(normal), _distance(distance)
{
}

template <std::size_t N, typename T>
plane<N, T>::plane(const vector<N + 1, T>& v)
{
  for (std::size_t i = 0; i < N; ++i)
    _normal(i) = v(i);
  _distance = v(N);
}

template <std::size_t N, typename T>
plane<N, T>::plane(const plane<N, T>& other)
: _normal(other._normal), _distance(other._distance)
{
}

template <std::size_t N, typename T>
plane<N, T>::plane(const vector3<T>& a, const vector3<T>& b,
                   const vector3<T>& c)
{
  _normal = ((a - b) % (a - c)).normalize();
  _distance = -a * _normal;
}

template <std::size_t N, typename T>
plane<N, T>& plane<N, T>::operator=(const plane& other)
{
  _normal = other._normal;
  _distance = other._distance;
  return *this;
}

template <std::size_t N, typename T>
plane<N, T> plane<N, T>::operator-()
{
  plane result(*this);
  return result.negate();
}

template <std::size_t N, typename T>
plane<N, T>& plane<N, T>::negate()
{
  _distance = -_distance;
  _normal.negate();
  return *this;
}

template <std::size_t N, typename T>
plane<N, T>& plane<N, T>::normalize()
{
  using std::abs;

  T magnitude = abs(_normal);
  BOOST_ASSERT(magnitude > 0);
  if (magnitude != 0)
  {
    _normal /= magnitude;
    _distance /= magnitude;
  }
  else
  {
    _normal = vector<N, T>{0, 0, 1};
    _distance = 1;
  }
  return *this;
}

/// @return
///    Returns a normalized copy of the input plane.
template <std::size_t N, typename T>
plane<N, T> normalize(const plane<N, T>& input)
{
  plane<N, T> result(input);
  return result.normalize();
}

/// Returns the signed distance of the passed point to the plane.
template <std::size_t N, typename T>
T distance(const plane<N, T>& plane, const vector<N, T>& point)
{
  return point * plane.normal() + plane.distance();
}

/////
// template <std::size_t N, std::size_t Columns, typename T>
// inline plane<N, T> operator*(const matrix<N, Columns, T>& lhs,
//                             const plane<N, T>& rhs)
//{
//  auto p = rhs._normal * rhs._distance;
//  auto p2 = lhs * rhs;
//  plane<N, T> result;
//  result._distance = std::abs(p2);
//  if (std::abs(result._distance) > epsilon<T>)
//    result._normal = p2 / result._distance;
//  return result;
//}

///
template <std::size_t N, typename T>
inline plane<N, T> operator*(const matrix<N + 1, N + 1, T>& lhs,
                             const plane<N, T>& rhs)
{
  return plane<N, T>{lhs * make_vector_from(rhs.normal(), rhs.distance())};
}
}

#endif
