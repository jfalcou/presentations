#include <print>
#include <sstream>
#include <kyosu/kyosu.hpp>

// kyosu prints through operator<<: a string for std::print
auto str(auto const& v) { std::ostringstream s; s << v; return s.str(); }

int main()
{
  auto pi   = eve::pi(eve::as<double>());
  auto zero = kyosu::complex(0., 0.), one = kyosu::complex(1., 0.), c = kyosu::complex(3., 4.);
  auto row  = [](auto n, auto const& a, auto const& b) { std::println("{:<16}{:<20}{}", n, str(a), str(b)); };

  row("option", "without", "with");

  row("real_only", kyosu::sqrt(-1.0), kyosu::sqrt[kyosu::real_only](-1.0)); // A complex result, or NaN

  row("raw", kyosu::log(zero), kyosu::log[kyosu::raw](zero));               // IEEE 754 edge cases, or the bare formula

  auto turns = 1e6 * one;
  row("radpi", kyosu::sin(pi * turns), kyosu::sin[kyosu::radpi](turns));    // pi carries a rounding, half turns do not

  row("flat", kyosu::abs(c), kyosu::abs[kyosu::flat](c));                   // The modulus, or the largest component
}
