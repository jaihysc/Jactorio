# Code Style

Outlined sections

* Naming
* Doc Comments

## Naming

### Code

```c++
//
// The file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: mm/dd/yyyy

#include "quotes/for/forward/declaration.h"  // If applicable

#include <external/std_lib>
#include <external/library.h>

#include "quotes/for/other/project/files.h"

#define MACRO_IN_UPPER 10


using big_double = long double;

const float constant_value = 3.14159265f;

namespace a::namespace_identifier
{
    enum class aEnumInCamel
    {
        enumerates_in_snake = 0,
        lower_case
        count_  // Notice the _ after count indicating the count of the enum
    }


    class Some_class : public Another_class
    {
    private:
        int private_member_;

        void private_method();
    public:
        int public_member;

        void public_method();
    }
}

void function_name() {
    const int value_1 = MACRO_IN_UPPER;
    int value_2 = 1 + value_1;

    auto value_3 = value_2;
    printf("%d", value_2);
}
```

### Files

```c++
some_compiled.h
some_compiled.cpp
some_compiledTests.cpp
```

## Doc Comments

Documentation comments are prefixed with triple slashes `///` per line. Since Visual Studio only picks up `///` for separate highlighting.

Doxygen tags are used with a prefix of `\`.

A `\brief` should always be provided for functions and methods, with one empty line at the beginning. Whereas variables may only use a single line of documentation comment omitting `\brief`.

```c++
/// Side b length in meters
constexpr double tri_b = 12.4;

///
/// \brief Calculates right angle triangle hypotenuse given sides a, b
double hypotenuse(double a, double b) {
    // Normal comment
    return sqrt(pow(a, 2) + (b, 2));
}
```
