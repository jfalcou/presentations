# std::complex Ate My Performances! - Making Faster Hyper-complex Maths

Meeting C++ 2026, in English, 60 minutes, 42 slides in six parts. The deck is `main.md.html`, and
its content is the reference: this plan lists the slides, their sources, and what each one lacks.

Sources on disk:

- kyosu `main` at `87741131`, EVE pinned at `829b03d99` (2026-10-31), kumi `v5.0` with `main` at
  `f2f3c73f`.
- `kyosu/doc/math.md`, `kyosu/doc/setup.md`, `kyosu/doc/rotation.svg`, the tutorials of
  `kyosu/doc/tutorial/`, the examples of `kyosu/test/doc/`, and `kyosu/demo/mandelbrot.cpp`.
- `bench-std.json` of the kumi drawer, for the compile time of kumi against `std::tuple`.
- P2663R7 and the working draft, [simd.complex.access] and [simd.complex.math].
- `code/`: the reference kernels, the snippets of the slides as complete programs, the Mandelbrot bench, and
  their scripts. The outputs of an i7-11700K are in `code/results/`.
- `review.md`: the four reviews of the first version, and the decisions they leave.

Status of each slide:

- *ready*: the content is final until a decision of `review.md` changes it.
- *to source*: a claim on the standard, a compiler, or a library, to back with its text or a
  Compiler Explorer link.
- *to measure*: a figure that no measure on disk holds yet.

## Title slide

- *std::complex Ate My Performances!*, *Making Faster Hyper-complex Maths*, Meeting C++ 2026.

# Part 1: Complex arithmetic in C++

| Slide | Content | Status |
|---|---|---|
| Hypercomplex numbers in computing | the domains, one reference each, the complex plane and the rotation drawings | ready |
| Hypercomplex types in C++ | type, header, and version in nine libraries | ready, each header read at its release |
| Reference kernels | caxpy, cdotc, and exp, the caxpy loop | ready, the kyosu versions to write |
| Constraints of std::complex | value type, layout, product, in one commented listing | ready, the draft, the GCC manual, and the code generation of libstdc++, libc++, and MSVC |
| Outline | the five parts, two lines each | ready |

# Part 2: Cayley-Dickson algebras

| Slide | Content | Status |
|---|---|---|
| Cayley-Dickson doubling | Hamilton and Dickson, the formula, the table of the tower | ready |
| Properties of the algebras | the properties lost, Bott and Milnor, Kervaire | ready |

# Part 3: Introducing kyosu

| Slide | Content | Status |
|---|---|---|
| kyosu | purpose and availability | ready |
| One name for every dimension | `kyosu::sqr` in dimensions 2, 4, and 8, from the README | ready |
| Mixed dimensions and divisions | the second example of the README | ready, its labels `li lj lk` follow the kyosu item on `il jl kl` |
| Options | the four options, one call each with its output | ready, outputs from `code/snippets/options.cpp` |
| Function catalog | ten families, their functions | ready |
| Extension of real analytic functions | motivation, the two steps, the code and its output, the hypotheses | ready, `extend` equals `kyosu::exp` on a quaternion |

# Part 4: From algebras to tuples

| Slide | Content | Status |
|---|---|---|
| Two representations of the tower | nested pairs against a flat tuple | ready, the case against nested pairs to argue |
| kumi | compile time against `std::tuple`, the chart at 64 elements | ready |
| kumi algorithms | `map`, `fold_left`, `transpose`, `cartesian_product`, a record | ready, built and run with g++ 14 |
| kyosu::cayley_dickson | the storage and the tuple protocol | ready |
| Product by split and cat | `operator*=`, `compounds.hpp:68-91` | ready |
| Mixed dimensions | the mixed branch, `compounds.hpp:92-102` | ready |
| Algorithms on components | `operator-` and `operator+=` | ready |

# Part 5: SIMD support

| Slide | Content | Status |
|---|---|---|
| Registers of complex numbers | EVE and its link, a register of `complex_t`, its output | ready |
| Register of a tuple | `find_register_type`, the diagram of the two layouts | ready |
| kyosu and std::simd | the two designs side by side, the state of std::simd | ready, P2663R7 adopted, libstdc++ 16 and libc++ checked |
| One source, scalar or register | the product on registers, the `energy` template, its output | ready |
| Data in memory | the deinterleave of interleaved data, separate arrays | the `soa_vector` example to write |

# Part 6: Benchmarks and live demo

| Slide | Content | Status |
|---|---|---|
| Assembly of the kyosu caxpy | the kyosu caxpy on Compiler Explorer | ready |
| Timings of the reference kernels | std::complex and kyosu on caxpy, cdotc, and exp, three instruction sets, with speedups | measured on an i7-11700K, `code/results/kernels-i7-11700k.txt` |
| Comparison table | six libraries | ready, each cell read at its version |
| Live Demo | the title alone, dark green on gold | the build on the laptop, a recording as fallback |
| Escape-time kernel | the three Mandelbrot loops, the same points | ready |
| Divergent lanes | the mask, the exit, the fixed `render_wide` | ready |
| Mandelbrot timings | i7-11700K, SSE4.2, AVX2, and AVX-512, the chart | ready, from `code/results/mandelbrot-i7-11700k.txt` |

# Closing

| Slide | Content | Status |
|---|---|---|
| Conclusion | std::complex and kyosu | ready |
| Resources | kyosu, EVE, kumi, and the code of the talk | the Compiler Explorer links and the Discord to add |
| Questions | | ready |

# Awaiting a decision

- the argument, items 1 to 9 of `review.md`, item 4 being on the deck since the slide on std::simd.
- the scope of the measures, given the date of the talk.
- the title, kept as given: English writes *Performance* and *Math*.
