//
//  Mesh.h
//
//
//  Created by Andrew Owens on 2015-10-25.
//
//

#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <vector>
#include <cstddef>

#include "Vec3f.h"

class Mesh {

public: // Helper structures
  struct Vertex {
  public:
    Vertex(Vec3f const &pos = Vec3f(), Vec3f const &rgb = Vec3f(1.f, 1.f, 1.f))
        : pos(pos), rgb(rgb) {}

    static constexpr size_t positionOffset() {
      return offsetof(struct Vertex, pos);
    }
    static constexpr size_t rgbOffset() { return offsetof(struct Vertex, rgb); }

    Vec3f pos;
    Vec3f rgb;
  };

  struct Triangle {
  public:
    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}

    int a, b, c;
  };

  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Triangle> Triangles;

public:
  Mesh() {}
  Mesh(Vertices const &verts, Triangles const &tris)
      : m_verts(verts), m_tris(tris){};

  Vertex const *vertexData() const { return m_verts.data(); }
  Triangle const *triangleData() const { return m_tris.data(); }

  size_t triangleCount() const { return m_tris.size(); }
  size_t indiceCount() const { return m_tris.size() * 3; }
  size_t vertexCount() const { return m_verts.size(); }

  Vertices const &vertices() const { return m_verts; }
  Vertices &vertices() { return m_verts; }

  typedef Triangles::const_iterator TrianglesIterator;
  TrianglesIterator trianglesBegin() const { return m_tris.begin(); }
  TrianglesIterator trianglesEnd() const { return m_tris.end(); }

private:
  Vertices m_verts;
  Triangles m_tris;
};

#endif /* defined(____Mesh__) */
