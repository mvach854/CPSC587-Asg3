#ifndef SPRING_H
#define SPRING_H

#include <iostream>
#include "Mass.h"

// Defines the properties of a Spring

class Spring {
public:
  Spring(float s, Mass A, Mass B, float r);

public:


private:
  float stiffness;
  Mass* massA;
  Mass* massB;
  float restLength;
};

#endif // SPRING_H
