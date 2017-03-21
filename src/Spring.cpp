#include "Spring.h"

// ======================== CONSTRUCTORS ============================//
Spring::Spring(float s, Mass* A, Mass* B, float  r) {
  stiffness = s;
  massA = A;
  massB = B;
  restLength = r;
}

// ==========================================================================//

// ========================= OPERATORS ======================================//
int Spring::getSize() {
  return Springs.size();
}
// ==========================================================================//
