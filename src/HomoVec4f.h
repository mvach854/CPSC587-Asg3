//
//  HomoVec4f.h
//
//
//  Created by Andrew Owens on 2015-04-08.
//
//	Helper class to do Mat4f multipilication with Vec3f

#ifndef HOMO_VEC4F_H
#define HOMO_VEC4F_H

#include <iostream>

#include "Mat4f.h"
#include "Vec3f.h"

class HomoVec4f {
public:
  explicit HomoVec4f(float x = 0.f, float y = 0.f, float z = 0.f,
                     float w = 1.f);
  explicit HomoVec4f(Vec3f const &p, float w = 1.f);

  operator Vec3f() const; // type conversion

  Vec3f perspectiveDivided() const;

  float operator[](int i) const;
  float &operator[](int i);

private:
  union {
    struct {
      float m_x, m_y, m_z, m_w;
    };
    float m_coord[4];
  };
};

inline Vec3f HomoVec4f::perspectiveDivided() const {
  Vec3f v = *this;
  v /= m_w;

  return v;
}

inline HomoVec4f::HomoVec4f(float x, float y, float z, float w)
    : m_x(x), m_y(y), m_z(z), m_w(w) {}

inline HomoVec4f::HomoVec4f(Vec3f const &p, float w)
    : m_x(p.x()), m_y(p.y()), m_z(p.z()), m_w(w) {}

inline float HomoVec4f::operator[](int i) const { return m_coord[i]; }

inline float &HomoVec4f::operator[](int i) { return m_coord[i]; }

inline HomoVec4f::operator Vec3f() const { return Vec3f(m_x, m_y, m_z); }

// Mat4f multiplication //
inline HomoVec4f operator*(Mat4f const &m, HomoVec4f const &v) {
  HomoVec4f result;

  float element;
  for (int i = 0; i < Mat4f::DIM; ++i) {
    element = 0;
    for (int j = 0; j < Mat4f::DIM; ++j) {
      element += m(i, j) * v[j];
    }
    result[i] = element;
  }

  return result;
}

#endif
