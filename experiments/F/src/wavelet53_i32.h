#pragma once

#include <cstdint>
#include <vector>

namespace f {

// int32 working versions for the lossy path (avoids int16 overflow on inverse)
void dwt53_forward_frame_i32(std::vector<int32_t>& frame, int width, int height, int levels);
void dwt53_inverse_frame_i32(std::vector<int32_t>& frame, int width, int height, int levels);

} // namespace f
