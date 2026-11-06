#pragma once

#include <cstdint>
#include <vector>

namespace f {

// Multi-level reversible 5/3. levels >= 1.
// Operates in-place on a width x height buffer with given stride.

void dwt53_forward_2d_levels(int16_t* data, int width, int height, int stride, int levels);
void dwt53_inverse_2d_levels(int16_t* data, int width, int height, int stride, int levels);

void dwt53_forward_frame(std::vector<int16_t>& frame, int width, int height, int levels);
void dwt53_inverse_frame(std::vector<int16_t>& frame, int width, int height, int levels);

// Subband rectangle after `levels` decompositions (LL is level 0).
struct Subband {
  int x, y, w, h;
  int level;   // 0 = LL, 1..levels = detail bands
  char type;   // 'L','H','V','D'
};

std::vector<Subband> list_subbands(int width, int height, int levels);

} // namespace f
