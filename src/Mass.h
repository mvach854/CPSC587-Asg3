#ifndef MASS_H
#define MASS_H

#include <iostream>
#include "Vec3f.h"

// Defines the properties of a Mass

class Mass {
public:
  Mass(float m, Vec3f pos);

private:
  float mass;
  Vec3f position;
  Vec3f velocity;
  Vec3f force;
};

#endif // MASS_H
