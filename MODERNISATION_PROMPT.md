Review this C++ codebase for remaining C idioms and inconsistent C++20 usage.

## Part A — C-to-C++20 conversions

For **every source file** (headers and .cpp), check for each of the following. Do not skip files.

### Types and casts

1. `unsigned char` → `uint8_t` (or other fixed-width `<cstdint>` type)
2. C-style casts `(type)expr` → `static_cast<type>(expr)` (or `reinterpret_cast` / `const_cast` as appropriate)
3. `typedef` → `using` alias
4. `UINT32_MAX` / `INT32_MAX` / `SIZE_MAX` → `std::numeric_limits<T>::max()`
5. `sizeof(array)/sizeof(array[0])` → `.size()` on `std::array` or `std::span`

### Memory and ownership

6. C arrays `type name[N]` → `std::array<type, N>`
7. Raw pointer locals that could be references (pointer is never null at point of use)
8. Pointer arithmetic (`p + n`, `p - q`, `p < q` on raw pointers) → index-based access with `[]`, iterators, or `.subspan()`
9. Raw pointer function parameters where `std::span` would work
10. `malloc` / `free` / `realloc` → `std::vector`, `std::make_unique`, or RAII container
11. Raw `new` / `delete` → `std::make_unique<T>()`
12. `const char*` string literals or parameters → `std::string_view`
13. `memcpy` / `memmove` → `std::copy`, `std::copy_n`, `std::copy_backward`
14. `memset` → `std::fill`, `std::fill_n`, or aggregate initialisation `{}`
15. `memcmp` for struct equality → `operator==() = default`

### Control flow and expressions

16. `!!expr` → `expr != 0` or `to_underlying(expr) != 0`
17. `(void)var` to suppress unused warnings → `[[maybe_unused]]` attribute
18. Missing `[[fallthrough]]` on intentional switch fall-throughs
19. Missing `[[nodiscard]]` on functions returning success/failure bools or non-trivial values
20. `abs()` → `std::abs()`
21. Manual min/max clamping → `std::clamp`, `std::min`, `std::max`
22. Manual bit counting / trailing zero → `std::popcount`, `std::countr_zero` from `<bit>`

### Constants and naming

23. Magic numbers → named constants (any numeric literal that isn't 0, 1, -1, or a character literal, and isn't already a named constant at the usage site — standard protocol codes like SGR numbers are exempt). Do not name a constant that is only used on the immediately adjacent lines — naming it adds verbosity without improving clarity (e.g. bit masks in a single expression, shift amounts in a local block).
24. `0xFF` / `0xFFFF` / `UINT32_MAX` sentinel values → named constants
25. Large numeric literals without digit separators → add separators (e.g. `100'000'000`, `0x03'FFFF`)
26. `#define` for constants → `inline constexpr`
27. `#define` for inline functions → `constexpr` functions
28. Duplicate constants defined in multiple translation units → consolidate into a single shared header

### Functions and templates

29. `static inline` functions → `constexpr` for pure computation
30. Redundant `static` on `constexpr` variables/arrays inside anonymous namespaces
31. Pure value-copying/mapping functions not marked `constexpr` → add `constexpr` where the function body permits it
32. Unconstrained templates that should only accept specific types → add `requires` clause (e.g. `requires std::is_enum_v<E>`, `requires std::invocable<F, Args...>`, `requires std::same_as<T, U>`)
33. Output parameters (pointer/reference written to by callee) that could be return values → return by value instead
34. Alias functions that are identical to the function they wrap → remove and replace call sites
35. `sprintf` / `snprintf` → `std::format`
36. C function pointer callbacks → virtual base class callbacks
37. If-else chains mapping ranges to values → `constexpr` lookup tables or `constexpr` lambda IIFE table construction
38. `#ifdef` compile-time branching in templates → `if constexpr`

### Structs and classes

39. Struct/class members without default initialisers → add `= 0`, `= {}`, etc. to prevent uninitialised-read bugs (including per-field defaults on bitfield members)
40. Pointer-to-nullable used as bool → explicit sentinel or dedicated flag
41. Manual comparison operators → `auto operator<=>(const T&) const = default` or `operator==() = default`
42. Non-copyable types missing deleted copy operations → add `= delete`
43. Implementation-detail types (classes, structs) visible at file scope in `.cpp` files → move into anonymous namespace
44. Repeated `static_cast<underlying>(enum_val)` chains → `to_underlying()` helper

### Style

45. Mixed comment styles (`/* */` vs `//`) within a file → use `//` consistently (except for file-level doc blocks)
46. Unscoped `enum` → `enum class`
47. Designated initializers: prefer `.field = value` syntax for aggregate types where it improves readability

### Tests

48. Test assertions comparing `bool` fields to integer literals (`0`/`1`) → use `true`/`false`

### Safety

49. Unchecked nullable pointer dereferences (pointer from lookup that can return null used without null check) → add null guard
50. Compile-time invariants enforced at runtime → `static_assert` where possible

### Catch-all

51. Any other pattern where a C++20 equivalent is cleaner

## Part B — Established idiom consistency checks

The codebase uses the following C++20 idioms. Check that they are applied **consistently** everywhere they should be, not just in some files.

1. **`inline constexpr` for header-defined constants** — all named constants in headers use `inline constexpr`, not `static constexpr` or `#define`.
2. **`enum class` for all enumerations** — no unscoped enums.
3. **`std::array` for all fixed-size collections** — no C arrays `T name[N]`.
4. **`std::span` for non-owning contiguous views** — function parameters that receive pointer+length pairs use `std::span`.
5. **`std::string_view` for non-owning string views** — string parameters that don't need ownership use `std::string_view`.
6. **`std::unique_ptr` for sole ownership** — no raw owning pointers; pImpl uses `std::unique_ptr<Impl>`.
7. **`std::make_unique`** — no raw `new` expressions.
8. **`std::format` / `std::format_string`** — no `sprintf`, `snprintf`, or manual string concatenation for formatted output.
9. **`std::to_array<>`** — constexpr array initialisation from braced lists (e.g. unicode tables).
10. **Constexpr lambda IIFE** — lookup tables built at compile time via `[]() constexpr { ... }()` pattern.
11. **`constexpr` functions** — all pure computation functions are `constexpr`.
12. **`noexcept` on operator overloads** — all arithmetic/bitwise/comparison operator overloads are `noexcept`.
13. **`[[nodiscard]]` on non-void returns** — success/failure bools, lookup functions, and pure getters have `[[nodiscard]]`.
14. **`[[maybe_unused]]`** — intentionally unused parameters use `[[maybe_unused]]`, not `(void)`.
15. **`[[fallthrough]]`** — intentional switch fall-throughs have `[[fallthrough]]`.
16. **`requires` clauses** — templates are constrained with `requires` using standard concepts (`std::invocable`, `std::same_as`, `std::is_enum_v`, `std::is_object_v`).
17. **`if constexpr`** — compile-time branching in templates uses `if constexpr`, not `#ifdef` or tag dispatch.
18. **`static_assert`** — compile-time invariants (e.g. enum ordering) use `static_assert`.
19. **Defaulted comparisons** — `auto operator<=>(const T&) const = default` and `operator==() = default` where applicable.
20. **Bitfield per-field defaults** — every bitfield member has an explicit `= 0` or `= EnumType::Value` default.
21. **Designated initializers** — aggregate types use `.field = value` for clarity.
22. **Digit separators** — large numeric literals use `'` separators (e.g. `100'000'000`, `0x03'FFFF`).
23. **`std::numeric_limits<T>::max()`** — sentinel values use `std::numeric_limits`, not C macros.
24. **`std::countr_zero` / `std::popcount`** — bit operations use `<bit>` header functions.
25. **`std::clamp` / `std::min` / `std::max`** — no manual min/max/clamp expressions.
26. **`std::copy` / `std::copy_n` / `std::copy_backward`** — no `memcpy` / `memmove`.
27. **`std::fill` / `std::fill_n`** — no `memset`.
28. **Virtual ABC callbacks** — callback interfaces are abstract base classes with virtual destructors `= default`, not C function pointer structs.
29. **Deleted copy for non-copyable types** — types with `std::unique_ptr` members have `= delete` on copy operations.
30. **Anonymous namespaces** — all implementation-detail types and constants in `.cpp` files live in anonymous namespaces.
31. **`to_underlying()` helper** — enum-to-integer conversions use the project's `to_underlying()`, not repeated `static_cast`.
32. **`__VA_OPT__`** — variadic macros use `__VA_OPT__(,)` for comma elision, not `##__VA_ARGS__`.
33. **Named constants for control codes** — C0/C1 control bytes, protocol constants, palette indices, etc. are named constants in `internal.h`.

## Constraints

- Exclude `std::optional` and `std::variant` (performance decision).
- Target is C++20, not C++23.
- Test infrastructure files (`test/test.h`, `test/harness.h`) and test `.cpp` files: apply all patterns, but `const char*` is acceptable in C-string comparison macros (e.g. `ASSERT_STR_EQ`).
- Do not replace C++ idioms with C equivalents (e.g. do not replace `<iostream>` / `std::cerr` with `<cstdio>` / `fputs`).

## Completeness requirements

- **File-by-file enumeration**: For each source file, list every instance found with exact line numbers. Report files with zero findings explicitly ("no findings").
- **Grep verification**: After the initial review, grep the codebase for each mechanical pattern (C-style casts, `unsigned char`, `(void)`, `!!`, `static inline`, bare `abs(`, `#define` constants, `typedef`, `memcpy`, `memset`, `sprintf`, `UINT32_MAX`) and confirm zero remaining instances. Report the grep commands and results.
- **Negative reports required**: For each numbered pattern above (both Part A and Part B), report either (a) findings with file:line locations, or (b) "zero instances — confirmed by [method]".

## Output format

Present findings grouped by category with file locations and proposed before/after snippets.

Do not implement — wait for approval. After approval, implement all changes, build, and run tests after each phase.

## Build and test instructions

- Build with cmake: `cd build && cmake --build .`
- Run all tests: `./test/libvtermcpp-test` (expect 630 passing tests)
- The git repository root for commits is `source/thirdparty/libvtermcpp`.
