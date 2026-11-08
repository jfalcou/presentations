#include <complex>
#include <cstddef>

void caxpy(std::complex<float> a, std::complex<float> const* x, std::complex<float>* y, std::size_t n)
{
  for (std::size_t i = 0; i < n; ++i)
    y[i] = a * x[i] + y[i];
}
