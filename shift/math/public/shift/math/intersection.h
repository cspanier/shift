#ifndef SHIFT_MATH_INTERSECTION_H
#define SHIFT_MATH_INTERSECTION_H

#include "shift/math/line.h"
#include "shift/math/ray.h"
#include "shift/math/aabb.h"
#include "shift/math/sphere.h"
#include "shift/math/frustum.h"

namespace shift::math
{
///
enum class line_line_intersection
{
  none,
  point,
  parallel,
  collinear_disjoint,
  collinear_overlapping
};

enum class aabb_frustum_intersection
{
  outside = -1,
  intersect = 0,
  inside = 1
};

/// A collection of several intersection tests.
struct intersection
{
  /// Line-line intersection test.
  /// @param intersection
  ///   The intersection of the two lines. This parameter is only written if
  ///   it is not null and if there is a single intersection point.
  template <typename T, std::size_t N>
  static line_line_intersection line_line(const line<N, T>& line1,
                                          const line<N, T>& line2,
                                          vector<N, T>* intersection = nullptr)
  {
    // Checking if
    // p + t * r
    // q + u * s
    // is solvable for t and u
    auto p = line1.begin();
    auto r = line1.end() - line1.begin();
    auto q = line2.begin();
    auto s = line2.end() - line2.begin();
    auto pq = q - p;
    auto denom = cross(r, s);
    auto nom_t = cross(pq, s);
    if (denom == 0)
    {
      if (nom_t == 0)
      {
        auto v = dot(pq, r);
        auto w = -dot(pq, s);
        if ((0 <= v && v <= dot(r, r)) || (0 <= w && w <= dot(s, s)))
          return line_line_intersection::collinear_overlapping;
        else
          return line_line_intersection::collinear_disjoint;
      }
      else
        return line_line_intersection::parallel;
    }
    else
    {
      T t = nom_t / denom;
      T u = cross(pq, r) / denom;
      if (0 <= t && t <= 1 && 0 <= u && u <= 1)
      {
        if (intersection)
          *intersection = p + t * r;
        return line_line_intersection::point;
      }
      else
        return line_line_intersection::none;
    }
  }

  /// Point-rectangle intersection test.
  template <bool IncludeEdges, typename T>
  static bool point_rect(const vector2<T>& point, const rectangle<T>& area)
  {
    if (IncludeEdges)
    {
      return point.x >= area.min.x && point.x <= area.max.x &&
             point.y >= area.min.y && point.y <= area.max.y;
    }
    else
    {
      return point.x > area.min.x && point.x < area.max.x &&
             point.y > area.min.y && point.y < area.max.y;
    }
  }

  /// Rectangles-rectangle intersection test.
  template <bool IncludeEdges, typename T>
  static bool rect_rect(const rectangle<T>& lhs, const rectangle<T>& rhs)
  {
    if (IncludeEdges)
    {
      return !(lhs.min.x > rhs.max.x || lhs.max.x < rhs.min.x ||
               lhs.min.y > rhs.max.y || lhs.max.y < rhs.min.y);
    }
    else
    {
      return !(lhs.min.x >= rhs.max.x || lhs.max.x <= rhs.min.x ||
               lhs.min.y >= rhs.max.y || lhs.max.y <= rhs.min.y);
    }
  }

  /// Checks whether a point is within a triangle, and if so returns the
  /// barycentric coordinates of the point inside the triangle.
  /// @remarks
  ///   The function is based on the equation:
  ///   p = v0 + (v1 - v0) * u + (v2 - v0) * v
  ///   with 0 <= u <= 1, 0 <= v <= 1, and u + v <= 1, if p is inside the
  ///   triangle.
  template <bool IncludeEdges, typename T>
  static bool point_triangle(const vector2<T>& p, const vector2<T>& v0,
                             const vector2<T>& v1, const vector2<T>& v2,
                             vector2<T>* uv = nullptr)
  {
    auto edge1 = v1 - v0;
    auto edge2 = v2 - v0;
    auto area = cross(edge2, edge1);
    if (area < 0.00001f)
      return false;
    auto f = 1 / area;
    auto u = f * (cross(v0, v2) + cross(edge2, p));
    auto v = f * (cross(v1, v0) + cross(p, edge1));

    if ((IncludeEdges && (u >= 0) && (v >= 0) && (1 - u - v >= 0)) ||
        (!IncludeEdges && (u > 0) && (v > 0) && (1 - u - v > 0)))
    {
      if (uv)
      {
        uv->x = u;
        uv->y = v;
      }
      return true;
    }
    else
      return false;
  }

  /// Checks whether a point is within a triangle, and if so returns the
  /// barycentric coordinates of the point inside the triangle.
  /// @remarks
  ///   The function is based on the equation:
  ///   p = v0 + (v1 - v0) * u + (v2 - v0) * v
  ///   with 0 <= u <= 1, 0 <= v <= 1, and u + v <= 1, if p is inside the
  ///   triangle.
  template <bool IncludeEdges, typename T>
  static bool point_triangle(const vector2<T>& p, const vector2<T>& v0,
                             const vector2<T>& v1, const vector2<T>& v2,
                             T delta, vector2<T>* uv = nullptr)
  {
    auto edge1 = v1 - v0;
    auto edge2 = v2 - v0;
    auto area = cross(edge2, edge1);
    if (area < 0.00001f)
      return false;
    auto f = 1 / area;
    auto u = f * (cross(v0, v2) + cross(edge2, p));
    auto v = f * (cross(v1, v0) + cross(p, edge1));

    if ((IncludeEdges && (u >= -delta) && (v >= -delta) &&
         (1 - u - v >= -delta)) ||
        (!IncludeEdges && (u > -delta) && (v > -delta) && (1 - u - v > -delta)))
    {
      if (uv)
      {
        uv->x = u;
        uv->y = v;
      }
      return true;
    }
    else
      return false;
  }

  ///
  template <bool IncludeEdges, typename T>
  static bool line_triangle(const line<2, T>& l, const vector2<T>& v0,
                            const vector2<T>& v1, const vector2<T>& v2)
  {
    line_line_intersection result;
    result = line_line(l, line<2, T>(v0, v1));
    if (result == line_line_intersection::point ||
        (result == line_line_intersection::collinear_overlapping &&
         IncludeEdges))
    {
      return true;
    }

    result = line_line(l, line<2, T>(v1, v2));
    if (result == line_line_intersection::point ||
        (result == line_line_intersection::collinear_overlapping &&
         IncludeEdges))
    {
      return true;
    }

    result = line_line(l, line<2, T>(v2, v0));
    if (result == line_line_intersection::point ||
        (result == line_line_intersection::collinear_overlapping &&
         IncludeEdges))
    {
      return true;
    }

    return false;
  }

  ///
  template <bool IncludeEdges, typename T>
  static bool rect_triangle(const rectangle<T>& r, const vector2<T>& v0,
                            const vector2<T>& v1, const vector2<T>& v2)
  {
    // Test if any triangle point is within the rectangle.
    if (point_rect<IncludeEdges>(v0, r) || point_rect<true>(v1, r) ||
        point_rect<IncludeEdges>(v2, r))
    {
      return true;
    }

    auto max_min = make_vector_from(r.max.x, r.min.y);
    auto min_max = make_vector_from(r.min.x, r.max.y);

    // Test if any rectangle point is within the triangle.
    if (point_triangle<IncludeEdges>(r.min, v0, v1, v2) ||
        point_triangle<IncludeEdges>(r.max, v0, v1, v2) ||
        point_triangle<IncludeEdges>(min_max, v0, v1, v2) ||
        point_triangle<IncludeEdges>(max_min, v0, v1, v2))
    {
      return true;
    }

    // Finally test if any edges intersect.
    if (line_triangle<IncludeEdges>(line<2, T>(r.min, max_min), v0, v1, v2) ||
        line_triangle<IncludeEdges>(line<2, T>(r.max, max_min), v0, v1, v2) ||
        line_triangle<IncludeEdges>(line<2, T>(r.min, min_max), v0, v1, v2) ||
        line_triangle<IncludeEdges>(line<2, T>(r.max, min_max), v0, v1, v2))
    {
      return true;
    }

    return false;
  }

  ///
  template <typename T, std::size_t N>
  static bool ray_triangle(const ray<N, T>& ray, const vector<N, T>& v0,
                           const vector<N, T>& v1, const vector<N, T>& v2,
                           float& t)
  {
    auto edge1 = v1 - v0;
    auto edge2 = v2 - v0;
    auto h = cross(ray.direction, edge2);
    auto a = edge1 * h;
    if (a > -0.00001f && a < 0.00001f)
      return false;

    auto f = 1 / a;
    auto s = ray.origin - v0;

    auto u = s * h * f;
    if (u < 0 || u > 1)
      return false;

    auto q = cross(s, edge1);
    auto v = ray.direction * q;
    if (v < 0 || u + v > 1)
      return false;

    t = edge2 * q * f;
    return t > 0;
  }

  ///
  template <typename T, std::size_t N>
  static bool ray_plane(const ray<N, T>& ray, const plane<N, T> plane, float& t)
  {
    T denominator = ray.direction * plane.normal();
    if (denominator > -epsilon<T>)
      return false;
    t = (plane.normal() * -plane.distance() - ray.origin) * plane.normal() /
        denominator;
    return t >= 0;
  }

  ///
  template <typename T, std::size_t N>
  static bool ray_sphere(const ray<N, T>& ray, const sphere<N, T> sphere,
                         float& t)
  {
    const auto to_origin = ray.origin - sphere.center;
    const T a = ray.direction * ray.direction;
    const T b = ray.direction * to_origin * 2;
    const T c = to_origin * to_origin - sphere.radius * sphere.radius;

    T discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
      return false;
    T q = (b < 0) ? (-b - sqrtf(discriminant)) / 2.0f
                  : (-b + sqrtf(discriminant)) / 2.0f;
    T t0 = q / a;
    T t1 = c / q;
    if (t0 > t1)
      std::swap(t0, t1);
    if (t1 < 0)
      return false;

    t = (t0 < 0 || sphere.radius < 0) ? t1 : t0;
    return true;
  }

  ///
  template <typename T, std::size_t N>
  static aabb_frustum_intersection aabb_frustum(const aabb<N, T>& box,
                                                const frustum<N, T>& frustum)
  {
    auto result = aabb_frustum_intersection::inside;
    for (std::size_t side = 0; side < 2 * N; ++side)
    {
      const vector<N, T>& normal = frustum.planes[side].normal();
      vector<N, T> abs_normal;
      for (std::size_t i = 0; i < N; ++i)
        abs_normal(i) = std::abs(frustum.planes[side].normal()(i));

      auto d = dot(box.center, normal);
      auto r = dot(box.extent, abs_normal);
      if (d + r < -frustum.planes[side].distance())
        return aabb_frustum_intersection::outside;
      if (d - r < -frustum.planes[side].distance())
        result = aabb_frustum_intersection::intersect;
    }
    return result;
  }
};
}

#endif
