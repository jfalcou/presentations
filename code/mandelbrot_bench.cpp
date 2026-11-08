// The Mandelbrot kernels of kyosu/demo/mandelbrot.cpp, timed without a window, plus kyosu computing z * z
// where the demo calls kyosu::sqr(z), to separate the cost of the square from the cost of the register.
//
//   g++ -std=c++20 -O3 -march=x86-64-v3 -I<kyosu>/include -I<eve>/include mandelbrot_bench.cpp -o mandelbrot_bench
//   ./mandelbrot_bench [runs]
//
// A second build with -fcx-limited-range removes the Annex G recovery from the std::complex kernel alone.
#include <kyosu/kyosu.hpp>

#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <vector>

using real_t = float;
using wide_t = eve::wide<real_t>;
using scalar_t = kyosu::complex_t<real_t>;

constexpr auto lanes = eve::width_v<wide_t>;

struct view
{
  char const* name;
  double cx, cy, scale;
  int max_iter;
};

// Every kernel computes c in double and rounds it once, so that all of them iterate on the same points.
real_t abscissa(view const& v, int x, int w) { return real_t(v.cx + (x - w / 2) * v.scale); }
real_t ordinate(view const& v, int y, int h) { return real_t(v.cy + (y - h / 2) * v.scale); }

template<bool UseSqr> auto square(auto z)
{
  if constexpr (UseSqr) return kyosu::sqr(z);
  else return z * z;
}

void render_std(std::vector<float>& esc, int w, int h, int stride, view const& v)
{
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      std::complex<real_t> const c{abscissa(v, x, w), ordinate(v, y, h)};
      std::complex<real_t> z{0, 0};

      int n = 0;
      while (n < v.max_iter && std::norm(z) < real_t(4))
      {
        z = z * z + c;
        ++n;
      }

      esc[y * stride + x] = float(n);
    }
  }
}

template<bool UseSqr> void render_scalar(std::vector<float>& esc, int w, int h, int stride, view const& v)
{
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      auto const c = kyosu::complex(abscissa(v, x, w), ordinate(v, y, h));
      auto z = scalar_t{0, 0};

      int n = 0;
      while (n < v.max_iter && kyosu::sqr_abs(z) < real_t(4))
      {
        z = square<UseSqr>(z) + c;
        ++n;
      }

      esc[y * stride + x] = float(n);
    }
  }
}

template<bool UseSqr> void render_wide(std::vector<float>& esc, int w, int h, int stride, view const& v)
{
  for (int y = 0; y < h; ++y)
  {
    wide_t const im{ordinate(v, y, h)};

    for (int x = 0; x < w; x += lanes)
    {
      wide_t const re([&](auto i, auto) { return abscissa(v, x + int(i), w); });
      auto const c = kyosu::complex(re, im);

      auto z = kyosu::complex(wide_t(0), wide_t(0));
      wide_t n{0};

      for (int i = 0; i < v.max_iter; ++i)
      {
        auto alive = kyosu::sqr_abs(z) < real_t(4);
        if (eve::none(alive)) break;

        z = square<UseSqr>(z) + c;
        n = eve::inc[alive](n);
      }

      eve::store(n, &esc[y * stride + x]);
    }
  }
}

struct result
{
  double ms;
  double iterations;
};

result run(auto render, std::vector<float>& esc, int w, int h, int stride, view const& v, int runs)
{
  std::vector<double> ms(runs);

  render(esc, w, h, stride, v);
  for (auto& t : ms)
  {
    auto const t0 = std::chrono::steady_clock::now();
    render(esc, w, h, stride, v);
    auto const t1 = std::chrono::steady_clock::now();
    t = std::chrono::duration<double, std::milli>(t1 - t0).count();
  }
  std::ranges::sort(ms);

  // Columns past w in each row are padding written by the last register, and are not counted.
  double iterations = 0;
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) iterations += esc[y * stride + x];

  return {ms[ms.size() / 2], iterations};
}

int main(int argc, char** argv)
{
  int const runs = argc > 1 ? std::atoi(argv[1]) : 9;
  int const w = 960, h = 640;
  int const stride = ((w + lanes - 1) / lanes) * lanes;

  view const views[] = {{"full set", -0.75, 0.0, 3.5 / 960, 200},
                        {"zoom x10^4", -0.743643887, 0.131825904, 3.5 / 960 / 1e4, 2000}};

  struct kernel
  {
    char const* name;
    void (*render)(std::vector<float>&, int, int, int, view const&);
  };

  kernel const kernels[] = {{"std::complex", render_std},
                            {"kyosu scalar z*z", render_scalar<false>},
                            {"kyosu scalar sqr", render_scalar<true>},
                            {"kyosu wide z*z", render_wide<false>},
                            {"kyosu wide sqr", render_wide<true>}};

  std::ostringstream api;
  api << eve::current_api;
  std::printf("%s, %d lanes of float, median of %d runs\n", api.str().c_str(), int(lanes), runs);

  std::vector<float> esc(std::size_t(stride) * h);
  for (auto const& v : views)
  {
    std::printf("\n%s, %d iterations\n", v.name, v.max_iter);

    double reference = 0;
    for (auto const& k : kernels)
    {
      auto const r = run(k.render, esc, w, h, stride, v, runs);
      if (reference == 0) reference = r.ms;
      std::printf("  %-18s %9.2f ms  x%-6.2f %14.0f iterations\n", k.name, r.ms, reference / r.ms, r.iterations);
    }
  }
}
