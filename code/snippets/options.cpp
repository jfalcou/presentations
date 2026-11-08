#include <array>
#include <iostream>
#include <span>
#include <kyosu/kyosu.hpp>

int main()
{
  using c_t = kyosu::complex_t<double>;
  auto pi   = eve::pi(eve::as<double>());
  auto zero = c_t(0., 0.), one = c_t(1., 0.);

  // A complex result, or the real function and its NaN
  std::cout << kyosu::sqrt(-1.0) << "| "
            << kyosu::sqrt[kyosu::real_only](-1.0) << "\n";

  // IEEE 754 edge cases, or the bare formula
  std::cout << kyosu::log(zero) << "| "
            << kyosu::log[kyosu::raw](zero) << "\n";

  // Radians carry the rounding of pi, half turns do not
  std::cout << kyosu::sin(1e6 * pi * one) << "| "
            << kyosu::sin[kyosu::radpi](1e6 * one) << "\n";

  // A rotation that skips the normalization of q
  std::array<double, 3> axis{0., 0., 1.}, v{1., 0., 0.};
  auto q = kyosu::from_angle_axis(pi / 2, std::span(axis));
  auto w = kyosu::rotate_vec[kyosu::assume_unitary](q, std::span(v));
  std::cout << w[0] << " " << w[1] << " " << w[2] << "\n";
}
