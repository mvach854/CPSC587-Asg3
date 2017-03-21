#ifndef MASS_H
#define MASS_H

#include <iostream>
#include "Vec3f.h"
#include <vector>

using namespace std;

// Defines the properties of a Mass

class Mass {
public:
  vector<Mass> Masses;
  Mass() {};
  Mass(float m, Vec3f pos);
  Vec3f getPos();

private:
  float mass;
  Vec3f position;
  Vec3f velocity;
  Vec3f force;
};

#endif // MASS_H
