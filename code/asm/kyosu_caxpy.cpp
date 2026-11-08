#include <kyosu/kyosu.hpp>
#include <cstddef>

// One array per part: no shuffle on load or store
void caxpy(kyosu::complex_t<float> a, float const* xr, float const* xi, float* yr, float* yi, std::size_t n)
{
  using wf = eve::wide<float>;
  for (std::size_t i = 0; i + wf::size() <= n; i += wf::size())
  {
    auto x = kyosu::complex(eve::load(xr + i), eve::load(xi + i));
    auto y = kyosu::complex(eve::load(yr + i), eve::load(yi + i));
    auto r = a * x + y;
    eve::store(kyosu::real(r), yr + i);
    eve::store(kyosu::imag(r), yi + i);
  }
}
