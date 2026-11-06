#include "../src/rice_i32.h"
#include <iostream>
#include <vector>

int main() {
  const int N = 32 * 32; // one block
  std::vector<int32_t> zeros(N, 0);

  uint8_t k = 0;
  auto payload = f::rice_encode_i32_ctx(zeros.data(), zeros.size(), 32, 32, k);

  // Header size used in the lossy wrapper (bx,by,bw,bh,k,plen)
  const size_t header_bytes = 2+2+2+2+1+4; // 13 bytes

  std::cout << "All-zero block (" << N << " coefficients)\n";
  std::cout << "selected k     = " << int(k) << "\n";
  std::cout << "payload bytes  = " << payload.size() << "\n";
  std::cout << "payload bits   = " << payload.size() * 8 << "\n";
  std::cout << "header bytes   = " << header_bytes << "\n";
  std::cout << "total per block= " << (payload.size() + header_bytes) << "\n";
  std::cout << "bits per coeff = " << (payload.size() * 8.0 / N) << "\n";

  // Also show fixed-k=0 for comparison
  auto p0 = f::rice_encode_i32(zeros.data(), zeros.size(), 0);
  std::cout << "\nFixed k=0 payload bytes = " << p0.size() << "\n";

  return 0;
}
