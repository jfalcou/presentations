#include <iostream>
#include <kyosu/kyosu.hpp>

// f, defined on the complex numbers, extended to any Cayley-Dickson value z
auto extend(auto f, auto z)
{
  auto p = kyosu::pure(z);
  auto v = f(kyosu::complex(kyosu::real(z), kyosu::abs(p)));
  return kyosu::real(v) + kyosu::ipart(v) * kyosu::sign(p);
}

int main()
{
  auto q = kyosu::quaternion_t<double>(1., 2., 3., 4.);
  std::cout << extend(kyosu::exp, q) << "\n" << kyosu::exp(q) << "\n";
}
