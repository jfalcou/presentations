#include <print>
#include <sstream>
#include <kyosu/kyosu.hpp>

// kyosu prints through operator<<: a string for std::print
auto str(auto const& v) { std::ostringstream s; s << v; return s.str(); }

// The basis element e_k of the algebra T
template<typename T, int K> T e() { T z{}; kyosu::get<K>(z) = 1.; return z; }

int main()
{
  using q_t = kyosu::quaternion_t<double>;
  using o_t = kyosu::octonion_t<double>;
  using s_t = kyosu::as_cayley_dickson_n_t<16, double>;

  auto i = e<q_t, 1>(), j = e<q_t, 2>();                                       // Quaternions: i j != j i
  std::println("i j - j i          = {}", str(kyosu::commutator(i, j)));

  auto oi = e<o_t, 1>(), oj = e<o_t, 2>(), ol = e<o_t, 4>();                   // Octonions: (i j) l != i (j l)
  std::println("(i j) l - i (j l)  = {}", str(kyosu::associator(oi, oj, ol)));

  auto x = e<s_t, 1>() + e<s_t, 10>(), y = e<s_t, 4>() - e<s_t, 15>();         // Sedenions: x y = 0, x and y nonzero
  std::println("|x| |y|, |x y|     = {:g}, {:g}", kyosu::abs(x) * kyosu::abs(y), kyosu::abs(x * y));
}
