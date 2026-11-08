#include <iostream>
#include <kyosu/kyosu.hpp>

auto energy(auto z)
{
  return kyosu::sqr_abs(z) + kyosu::real(kyosu::exp(z));
}

int main()
{
  auto c = kyosu::complex_t<double>(1., 2.);
  std::cout << energy(c) << "\n";

  eve::wide<double> re = [](auto i, auto) { return 1.0 + i; };
  std::cout << energy(kyosu::complex(re, re)) << "\n";
}
