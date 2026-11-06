#include "wavelet53_i32.h"
#include <algorithm>
#include <vector>

namespace f {

static inline int32_t lift_predict32(int32_t a, int32_t b) { return (a + b) >> 1; }
static inline int32_t lift_update32(int32_t a, int32_t b) { return (a + b + 2) >> 2; }

static void forward_1d_i32(int32_t* data, int n) {
  if (n < 2) return;
  for (int i = 1; i < n - 1; i += 2)
    data[i] -= lift_predict32(data[i - 1], data[i + 1]);
  if (n % 2 == 0) data[n - 1] -= data[n - 2];
  for (int i = 2; i < n - 1; i += 2)
    data[i] += lift_update32(data[i - 1], data[i + 1]);
  if (n > 1) data[0] += (data[1] + 1) >> 1;
}

static void inverse_1d_i32(int32_t* data, int n) {
  if (n < 2) return;
  if (n > 1) data[0] -= (data[1] + 1) >> 1;
  for (int i = 2; i < n - 1; i += 2)
    data[i] -= lift_update32(data[i - 1], data[i + 1]);
  if (n % 2 == 0) data[n - 1] += data[n - 2];
  for (int i = 1; i < n - 1; i += 2)
    data[i] += lift_predict32(data[i - 1], data[i + 1]);
}

static void forward_2d_i32(int32_t* data, int w, int h, int stride) {
  for (int y = 0; y < h; ++y) forward_1d_i32(data + y * stride, w);
  std::vector<int32_t> col(h);
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) col[y] = data[y * stride + x];
    forward_1d_i32(col.data(), h);
    for (int y = 0; y < h; ++y) data[y * stride + x] = col[y];
  }
}

static void inverse_2d_i32(int32_t* data, int w, int h, int stride) {
  std::vector<int32_t> col(h);
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) col[y] = data[y * stride + x];
    inverse_1d_i32(col.data(), h);
    for (int y = 0; y < h; ++y) data[y * stride + x] = col[y];
  }
  for (int y = 0; y < h; ++y) inverse_1d_i32(data + y * stride, w);
}

void dwt53_forward_frame_i32(std::vector<int32_t>& frame, int width, int height, int levels) {
  int w = width, h = height;
  for (int lev = 0; lev < levels; ++lev) {
    forward_2d_i32(frame.data(), w, h, width);
    w = (w + 1) / 2;
    h = (h + 1) / 2;
  }
}

void dwt53_inverse_frame_i32(std::vector<int32_t>& frame, int width, int height, int levels) {
  std::vector<int> ws(levels + 1), hs(levels + 1);
  ws[0] = width; hs[0] = height;
  for (int i = 1; i <= levels; ++i) {
    ws[i] = (ws[i - 1] + 1) / 2;
    hs[i] = (hs[i - 1] + 1) / 2;
  }
  for (int lev = levels - 1; lev >= 0; --lev)
    inverse_2d_i32(frame.data(), ws[lev], hs[lev], width);
}

} // namespace f
