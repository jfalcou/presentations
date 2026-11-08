#include <complex>
#include <iostream>
#include <vector>
#include <kyosu/kyosu.hpp>

int main()
{
  using wf = eve::wide<float>;

  std::vector<std::complex<float>> x(wf::size());
  for (std::size_t k = 0; k < x.size(); ++k) x[k] = {float(k), 1.f};

  // x read as floats, through the access of [complex.numbers.general]/4
  float* p = reinterpret_cast<float*>(x.data());

  // Interleaved parts in, one register per part out
  auto [re, im] = eve::deinterleave_groups(eve::lanes<1>, eve::load(p), eve::load(p + wf::size()));
  auto z        = kyosu::sqr(kyosu::complex(re, im));

  // One register per part in, interleaved parts out
  auto [lo, hi] = eve::interleave(kyosu::real(z), kyosu::imag(z));
  eve::store(lo, p);
  eve::store(hi, p + wf::size());

  for (auto v : x) std::cout << v << ' ';
  std::cout << '\n';
}
