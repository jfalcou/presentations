#include <iostream>
#include <kumi/kumi.hpp>

int main()
{
  auto xs = kumi::tuple{1, 2.5f, 3.0};
  auto ys = kumi::tuple{'a', short{2}, 4ULL};

  auto sums  = kumi::map([](auto x, auto y) { return x + y; }, xs, ys);
  auto total = kumi::fold_left([](auto acc, auto v) { return acc + v; }, sums, 0.);
  std::cout << sums << " " << total << "\n";

  auto rows = kumi::tuple{kumi::tuple{1, 'a'}, kumi::tuple{2, 'b'}, kumi::tuple{3, 'c'}};
  std::cout << kumi::transpose(rows) << "\n";

  auto pairs = kumi::cartesian_product(kumi::tuple{1, 2}, kumi::tuple{'x', 'y'});
  std::cout << pairs << "\n";

  auto zs = kumi::flatten(kumi::tuple{kumi::tuple{1.5f, -2.f}, kumi::tuple{0.f, 4.f}});
  std::cout << zs << " " << kumi::all_of(zs, [](auto v) { return v < 5; }) << "\n";
}
