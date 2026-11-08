#include <iostream>
#include <eve/module/algo.hpp>
#include <kyosu/kyosu.hpp>

int main()
{
  using c_t = kyosu::complex_t<float>;

  // One array per part: each load fills one register per part, without a shuffle
  eve::algo::soa_vector<c_t> x;
  for (int k = 0; k < 8; ++k) x.push_back(c_t(float(k), 1.f));

  // The algorithm loads, computes, and stores a register at a time
  eve::algo::transform_inplace(x, [](auto z) { return kyosu::sqr(z); });

  for (std::size_t k = 0; k < x.size(); ++k) std::cout << x.get(k) << ' ';
  std::cout << '\n';
}
