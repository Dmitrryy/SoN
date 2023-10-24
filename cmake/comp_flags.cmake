set(SANITIZERS
    -fsanitize=address
    -fsanitize=alignment
    -fsanitize=bool
    -fsanitize=bounds
    -fsanitize=enum
    -fsanitize=float-cast-overflow
    -fsanitize=float-divide-by-zero
    -fsanitize=integer-divide-by-zero
    -fsanitize=leak
    -fsanitize=nonnull-attribute
    -fsanitize=null
    # -fsanitize=object-size
    -fsanitize=return
    -fsanitize=returns-nonnull-attribute
    -fsanitize=shift
    -fsanitize=signed-integer-overflow
    -fsanitize=undefined
    -fsanitize=unreachable
    -fsanitize=vla-bound
    -fsanitize=vptr
    -fsized-deallocation)

set(COMMON_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wno-gnu-zero-variadic-macro-arguments
    # -Weffc++
    -Wc++17-compat
    -Wcast-align
    -Wcast-qual
    -Wchar-subscripts
    -Wconversion
    -Wctor-dtor-privacy
    -Wempty-body
    -Wfloat-equal
    -Wformat-nonliteral
    -Wformat-security
    -Wformat=2
    -Winline
    # -Wlarger-than=8192
    -Wmissing-declarations
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wpacked
    -Wpointer-arith
    -Wredundant-decls
    -Wshadow
    -Wsign-conversion
    -Wsign-promo
    -Wstrict-overflow=2
    -Wsuggest-override
    -Wswitch-default
    -Wswitch-enum
    -Wundef
    -Wunreachable-code
    -Wunused
    -Wvariadic-macros
    -Wmissing-field-initializers
    -Wnarrowing
    -Wold-style-cast
    -Wvarargs)

set(GCC_WARNINGS
    -Wno-literal-suffix
    -Wuseless-cast
    -Wsync-nand
    -Wsuggest-final-methods
    -Wsuggest-final-types
    -Wsuggest-attribute=noreturn
    -Wstrict-null-sentinel
    -Wstack-usage=8192
    -Wopenmp-simd
    -Wlogical-op
    -Wconditionally-supported
    -Wformat-signedness
    -Waggressive-loop-optimizations
    -fcheck-new
    -fstack-protector
    -fstrict-overflow
    -flto-odr-type-merging
    -fno-omit-frame-pointer)

function(apply_compiler_flags TARGET VISIBILIY)
  # Add sanitizers
  target_link_options(${TARGET} ${VISIBILIY} "$<$<CONFIG:Debug>:${SANITIZERS}>")
  target_compile_options(${TARGET} ${VISIBILIY}
                         "$<$<CONFIG:Debug>:${SANITIZERS}>")

  # Compile stuff
  target_compile_options(${TARGET} ${VISIBILIY}
                         "$<$<CONFIG:Debug>:${COMMON_WARNINGS}>")
  target_compile_options(
    ${TARGET} ${VISIBILIY}
    "$<$<CXX_COMPILER_ID:GNU>:$<$<CONFIG:Debug>:${GCC_WARNINGS}>>")
endfunction()

string(REPLACE " " ";" DED_SAN_LST "${SANITIZERS}")
string(REPLACE " " ";" DED_GCC_WARNS_LST "${COMMON_WARNINGS}")
string(REPLACE " " ";" DED_GCC_WARNS_LST "${GCC_WARNINGS}")
