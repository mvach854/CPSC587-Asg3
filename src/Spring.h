#ifndef SPRING_H
#define SPRING_H

#include <iostream>
#include "Mass.h"
#include <vector>

using namespace std;

// Defines the properties of a Spring

class Spring {
public:
  vector<Spring> Springs;
  Spring() {};
  Spring(float s, Mass* A, Mass* B, float r);
  int getSize();

private:
  float stiffness;
  Mass* massA;
  Mass* massB;
  float restLength;
};

#endif // SPRING_H
