// Quaternion and octonion kernels of the talk, an axpy and an element-wise exp, over float: Boost.Math and kyosu one
// value at a time, then kyosu on registers, one array per component. Each is timed as the median of several runs, in
// ns per element.
//
//   g++ -std=c++20 -O3 -DNDEBUG -march=x86-64-v3 -Ikyosu/include -Ieve/include hypercomplex_kernels.cpp -o hypercomplex_kernels
//   ./hypercomplex_kernels [n] [runs]
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>
#include <boost/math/octonion.hpp>
#include <boost/math/quaternion.hpp>
#include <kyosu/kyosu.hpp>

using wf = eve::wide<float>;

template<typename Kernel> double median_ns_per_element(Kernel kernel, std::size_t n, int runs)
{
  std::vector<double> ns(runs);

  kernel();
  for (auto& t : ns)
  {
    auto const t0 = std::chrono::steady_clock::now();
    kernel();
    auto const t1 = std::chrono::steady_clock::now();
    t = std::chrono::duration<double, std::nano>(t1 - t0).count() / double(n);
  }

  std::ranges::sort(ns);
  return ns[ns.size() / 2];
}

// Component k of element i, in [0, 1)
float input(std::size_t i, int k) { return float((i * (k + 3)) % 17) / 17.f; }

// Boost.Math, one value at a time
template<int N> struct boost_type;
template<> struct boost_type<4> { using type = boost::math::quaternion<float>; };
template<> struct boost_type<8> { using type = boost::math::octonion<float>; };

template<int N> auto boost_value(auto f)
{
  if constexpr (N == 4) return boost::math::quaternion<float>(f(0), f(1), f(2), f(3));
  else return boost::math::octonion<float>(f(0), f(1), f(2), f(3), f(4), f(5), f(6), f(7));
}

// kyosu, one array per component, loaded and stored a register at a time
template<int N> using parts = std::array<std::vector<float>, N>;

template<int N> auto load(parts<N> const& a, std::size_t i)
{
  return [&]<std::size_t... K>(std::index_sequence<K...>)
  { return eve::wide<kyosu::as_cayley_dickson_n_t<N, float>>(eve::load(&a[K][i])...); }(std::make_index_sequence<N>{});
}

template<int N> void store(auto z, parts<N>& a, std::size_t i)
{
  [&]<std::size_t... K>(std::index_sequence<K...>)
  { (eve::store(eve::get<K>(z), &a[K][i]), ...); }(std::make_index_sequence<N>{});
}

template<int N> void run(std::size_t n, int runs)
{
  using bq = typename boost_type<N>::type;
  using kq = kyosu::as_cayley_dickson_n_t<N, float>;

  auto ka = kq{};
  kumi::for_each_index([&](auto k, auto& m) { m = (int(k) % 2 ? -0.5f : 0.5f) / float(int(k) + 1); }, ka);
  auto ba = boost_value<N>([&](int k) { return (k % 2 ? -0.5f : 0.5f) / float(k + 1); });

  std::vector<bq> bx(n), by(n), bz(n);
  std::vector<kq> kx(n), ky(n), kz(n);
  parts<N> px, py, pz;
  for (int k = 0; k < N; ++k) px[k].resize(n), py[k].resize(n), pz[k].resize(n);

  for (std::size_t i = 0; i < n; ++i)
  {
    bx[i] = boost_value<N>([&](int k) { return input(i, k); });
    by[i] = boost_value<N>([&](int k) { return input(i + 5, k); });
    kumi::for_each_index([&](auto k, auto& m) { m = input(i, int(k)); }, kx[i]);
    kumi::for_each_index([&](auto k, auto& m) { m = input(i + 5, int(k)); }, ky[i]);
    for (int k = 0; k < N; ++k) px[k][i] = input(i, k), py[k][i] = input(i + 5, k);
  }

  double const b_axpy = median_ns_per_element([&] { for (std::size_t i = 0; i < n; ++i) by[i] = ba * bx[i] + by[i]; }, n, runs);
  double const b_exp  = median_ns_per_element([&] { for (std::size_t i = 0; i < n; ++i) bz[i] = exp(bx[i]); }, n, runs);
  double const s_axpy = median_ns_per_element([&] { for (std::size_t i = 0; i < n; ++i) ky[i] = ka * kx[i] + ky[i]; }, n, runs);
  double const s_exp  = median_ns_per_element([&] { for (std::size_t i = 0; i < n; ++i) kz[i] = kyosu::exp(kx[i]); }, n, runs);
  double const w_axpy = median_ns_per_element([&] {
    for (std::size_t i = 0; i < n; i += wf::size()) store<N>(ka * load<N>(px, i) + load<N>(py, i), py, i);
  }, n, runs);
  double const w_exp = median_ns_per_element([&] {
    for (std::size_t i = 0; i < n; i += wf::size()) store<N>(kyosu::exp(load<N>(px, i)), pz, i);
  }, n, runs);

  std::printf("%s, n = %zu, median of %d runs, ns per element\n", N == 4 ? "quaternions" : "octonions", n, runs);
  std::printf("         Boost.Math  kyosu, scalar  kyosu, register\n");
  std::printf("  axpy  %10.3f  %13.3f  %15.3f\n", b_axpy, s_axpy, w_axpy);
  std::printf("  exp   %10.3f  %13.3f  %15.3f\n", b_exp, s_exp, w_exp);

  // The real part of the last exp of each implementation, then sums printed so that no kernel is removed as dead code.
  std::printf("exp %g %g %g\n", double(bz[n - 1].R_component_1()), double(kyosu::real(kz[n - 1])), double(pz[0][n - 1]));
  std::printf("checksum %g %g %g\n", double(by[n / 2].R_component_1()), double(kyosu::real(ky[n / 2])), double(py[0][n / 2]));
}

int main(int argc, char** argv)
{
  // The register kernels step by a whole register: n is a multiple of its width.
  std::size_t const n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 4096;
  int const runs = argc > 2 ? std::atoi(argv[2]) : 1001;

  run<4>(n, runs);
  run<8>(n, runs);
}
