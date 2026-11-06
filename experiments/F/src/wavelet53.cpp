#include "wavelet53.h"
#include <algorithm>
#include <vector>

namespace f {

static inline int16_t lift_predict(int16_t a, int16_t b) {
  return static_cast<int16_t>((a + b) >> 1);
}
static inline int16_t lift_update(int16_t a, int16_t b) {
  return static_cast<int16_t>((a + b + 2) >> 2);
}

static void dwt53_forward_1d(int16_t* data, int n) {
  if (n < 2) return;
  for (int i = 1; i < n - 1; i += 2)
    data[i] -= lift_predict(data[i - 1], data[i + 1]);
  if (n % 2 == 0) data[n - 1] -= data[n - 2];
  for (int i = 2; i < n - 1; i += 2)
    data[i] += lift_update(data[i - 1], data[i + 1]);
  if (n > 1) data[0] += (data[1] + 1) >> 1;
}

static void dwt53_inverse_1d(int16_t* data, int n) {
  if (n < 2) return;
  if (n > 1) data[0] -= (data[1] + 1) >> 1;
  for (int i = 2; i < n - 1; i += 2)
    data[i] -= lift_update(data[i - 1], data[i + 1]);
  if (n % 2 == 0) data[n - 1] += data[n - 2];
  for (int i = 1; i < n - 1; i += 2)
    data[i] += lift_predict(data[i - 1], data[i + 1]);
}

static void forward_2d_once(int16_t* data, int w, int h, int stride) {
  for (int y = 0; y < h; ++y) dwt53_forward_1d(data + y * stride, w);
  std::vector<int16_t> col(h);
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) col[y] = data[y * stride + x];
    dwt53_forward_1d(col.data(), h);
    for (int y = 0; y < h; ++y) data[y * stride + x] = col[y];
  }
}

static void inverse_2d_once(int16_t* data, int w, int h, int stride) {
  std::vector<int16_t> col(h);
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) col[y] = data[y * stride + x];
    dwt53_inverse_1d(col.data(), h);
    for (int y = 0; y < h; ++y) data[y * stride + x] = col[y];
  }
  for (int y = 0; y < h; ++y) dwt53_inverse_1d(data + y * stride, w);
}

void dwt53_forward_2d_levels(int16_t* data, int width, int height, int stride, int levels) {
  int w = width, h = height;
  for (int lev = 0; lev < levels; ++lev) {
    forward_2d_once(data, w, h, stride);
    w = (w + 1) / 2;
    h = (h + 1) / 2;
  }
}

void dwt53_inverse_2d_levels(int16_t* data, int width, int height, int stride, int levels) {
  // Reconstruct sizes from coarsest to finest
  std::vector<int> ws(levels + 1), hs(levels + 1);
  ws[0] = width; hs[0] = height;
  for (int i = 1; i <= levels; ++i) {
    ws[i] = (ws[i - 1] + 1) / 2;
    hs[i] = (hs[i - 1] + 1) / 2;
  }
  for (int lev = levels - 1; lev >= 0; --lev) {
    inverse_2d_once(data, ws[lev], hs[lev], stride);
  }
}

void dwt53_forward_frame(std::vector<int16_t>& frame, int width, int height, int levels) {
  dwt53_forward_2d_levels(frame.data(), width, height, width, levels);
}
void dwt53_inverse_frame(std::vector<int16_t>& frame, int width, int height, int levels) {
  dwt53_inverse_2d_levels(frame.data(), width, height, width, levels);
}

std::vector<Subband> list_subbands(int width, int height, int levels) {
  std::vector<Subband> sb;
  int w = width, h = height;
  for (int lev = 1; lev <= levels; ++lev) {
    int hw = w / 2, hh = h / 2;
    // LH, HL, HH at this level (relative to current LL origin which is 0,0)
    sb.push_back({hw, 0,  w - hw, hh, lev, 'H'}); // HL
    sb.push_back({0,  hh, hw, h - hh, lev, 'V'}); // LH
    sb.push_back({hw, hh, w - hw, h - hh, lev, 'D'}); // HH
    w = hw; h = hh;
  }
  // LL
  sb.push_back({0, 0, w, h, 0, 'L'});
  return sb;
}

} // namespace f
