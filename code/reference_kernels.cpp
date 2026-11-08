// Reference kernels of the talk, caxpy, cdotc, and an element-wise exp, over std::complex<float> and over
// eve::wide<kyosu::complex_t<float>>, each timed as the median of several runs, in ns per element.
// kyosu reads the std::complex arrays through a deinterleave, and separate arrays of parts directly.
//
//   g++ -std=c++20 -O3 -DNDEBUG -march=x86-64-v3 -Ikyosu/include -Ieve/include reference_kernels.cpp -o reference_kernels
//   ./reference_kernels [n] [runs]
#include <algorithm>
#include <chrono>
#include <complex>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <kyosu/kyosu.hpp>

using cvec = std::vector<std::complex<float>>;

void caxpy(std::complex<float> a, cvec const& x, cvec& y)
{
  for (std::size_t i = 0; i < x.size(); ++i)
    y[i] = a * x[i] + y[i];
}

std::complex<float> cdotc(cvec const& x, cvec const& y)
{
  std::complex<float> s{};
  for (std::size_t i = 0; i < x.size(); ++i)
    s += std::conj(x[i]) * y[i];
  return s;
}

void cexp(cvec const& x, cvec& y)
{
  for (std::size_t i = 0; i < x.size(); ++i)
    y[i] = std::exp(x[i]);
}

using wf = eve::wide<float>;

struct parts
{
  std::vector<float> re, im;
};

// Interleaved parts: a deinterleave on load, an interleave on store
auto load(std::complex<float> const* p)
{
  auto f       = reinterpret_cast<float const*>(p);
  auto [re, im] = eve::deinterleave_groups(eve::lanes<1>, eve::load(f), eve::load(f + wf::size()));
  return kyosu::complex(re, im);
}

void store(auto z, std::complex<float>* p)
{
  auto f        = reinterpret_cast<float*>(p);
  auto [lo, hi] = eve::interleave(kyosu::real(z), kyosu::imag(z));
  eve::store(lo, f);
  eve::store(hi, f + wf::size());
}

void kyosu_caxpy(kyosu::complex_t<float> a, cvec const& x, cvec& y)
{
  for (std::size_t i = 0; i < x.size(); i += wf::size())
    store(a * load(&x[i]) + load(&y[i]), &y[i]);
}

kyosu::complex_t<float> kyosu_cdotc(cvec const& x, cvec const& y)
{
  eve::wide<kyosu::complex_t<float>> s{};
  for (std::size_t i = 0; i < x.size(); i += wf::size())
    s += kyosu::conj(load(&x[i])) * load(&y[i]);
  return kyosu::complex(eve::reduce(kyosu::real(s)), eve::reduce(kyosu::imag(s)));
}

void kyosu_exp(cvec const& x, cvec& y)
{
  for (std::size_t i = 0; i < x.size(); i += wf::size())
    store(kyosu::exp(load(&x[i])), &y[i]);
}

// Separate arrays of parts: plain loads and stores
auto load(parts const& x, std::size_t i)
{
  return kyosu::complex(eve::load(&x.re[i]), eve::load(&x.im[i]));
}

void store(auto z, parts& y, std::size_t i)
{
  eve::store(kyosu::real(z), &y.re[i]);
  eve::store(kyosu::imag(z), &y.im[i]);
}

void kyosu_caxpy(kyosu::complex_t<float> a, parts const& x, parts& y)
{
  for (std::size_t i = 0; i < x.re.size(); i += wf::size())
    store(a * load(x, i) + load(y, i), y, i);
}

kyosu::complex_t<float> kyosu_cdotc(parts const& x, parts const& y)
{
  eve::wide<kyosu::complex_t<float>> s{};
  for (std::size_t i = 0; i < x.re.size(); i += wf::size())
    s += kyosu::conj(load(x, i)) * load(y, i);
  return kyosu::complex(eve::reduce(kyosu::real(s)), eve::reduce(kyosu::imag(s)));
}

void kyosu_exp(parts const& x, parts& y)
{
  for (std::size_t i = 0; i < x.re.size(); i += wf::size())
    store(kyosu::exp(load(x, i)), y, i);
}

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

int main(int argc, char** argv)
{
  std::size_t const n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : std::size_t(1) << 20;
  int const runs = argc > 2 ? std::atoi(argv[2]) : 31;

  // Inputs stay in [0, 1) on both parts, so neither exp nor caxpy meets an infinity or a subnormal.
  cvec x(n), y(n), z(n);
  for (std::size_t i = 0; i < n; ++i)
  {
    x[i] = {float(i % 17) / 17.f, float(i % 13) / 13.f};
    y[i] = {float(i % 11) / 11.f, float(i % 7) / 7.f};
  }

  std::complex<float> const a{0.5f, -0.25f};
  std::complex<float> dot{};

  double const t_axpy = median_ns_per_element([&] { caxpy(a, x, y); }, n, runs);
  double const t_dotc = median_ns_per_element([&] { dot += cdotc(x, y); }, n, runs);
  double const t_exp = median_ns_per_element([&] { cexp(x, z); }, n, runs);

  // The kernels of kyosu step by a whole register: n is a multiple of its width.
  cvec ky = y, kz(n);
  parts px{std::vector<float>(n), std::vector<float>(n)}, py = px, pz = px;
  for (std::size_t i = 0; i < n; ++i)
  {
    px.re[i] = x[i].real(), px.im[i] = x[i].imag();
    py.re[i] = ky[i].real(), py.im[i] = ky[i].imag();
  }
  kyosu::complex_t<float> const ka{0.5f, -0.25f};
  kyosu::complex_t<float> kdot{}, pdot{};

  double const k_axpy = median_ns_per_element([&] { kyosu_caxpy(ka, x, ky); }, n, runs);
  double const k_dotc = median_ns_per_element([&] { kdot += kyosu_cdotc(x, ky); }, n, runs);
  double const k_exp  = median_ns_per_element([&] { kyosu_exp(x, kz); }, n, runs);
  double const p_axpy = median_ns_per_element([&] { kyosu_caxpy(ka, px, py); }, n, runs);
  double const p_dotc = median_ns_per_element([&] { pdot += kyosu_cdotc(px, py); }, n, runs);
  double const p_exp  = median_ns_per_element([&] { kyosu_exp(px, pz); }, n, runs);

  std::printf("n = %zu, median of %d runs, ns per element\n", n, runs);
  std::printf("         std::complex  kyosu, interleaved  kyosu, separate parts\n");
  std::printf("  caxpy  %12.3f  %18.3f  %21.3f\n", t_axpy, k_axpy, p_axpy);
  std::printf("  cdotc  %12.3f  %18.3f  %21.3f\n", t_dotc, k_dotc, p_dotc);
  std::printf("  exp    %12.3f  %18.3f  %21.3f\n", t_exp, k_exp, p_exp);

  // The exp of both libraries on one input, then sums printed so that no kernel is removed as dead code.
  std::printf("exp %g %g | %g %g\n", double(z[n - 1].real()), double(z[n - 1].imag()), double(kz[n - 1].real()),
              double(kz[n - 1].imag()));
  std::printf("checksum %g %g %g %g %g\n", double(y[n / 2].real() + dot.real()), double(dot.imag()),
              double(kyosu::real(kdot)), double(kyosu::real(pdot)), double(pz.re[n - 1] + ky[n / 2].real()));
}
