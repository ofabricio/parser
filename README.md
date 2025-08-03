A text parser in C++.

## Example

This example shows how to collect the names and the arguments of the statements.

```cpp
#include <cstdio>
#include "parser.hpp"

int main()
{
    Parser p("point(1 20)\n"
             "vector(-2 -30)");

    while (p.More()) {
        auto m = p.Mark();
        if (p.While({ 'a', 'z' })) {
            auto tok = p.Token(m);
            int x, y;
            if (p.Match('(') && p.NumberOut(&x) && p.Space() && p.NumberOut(&y) && p.Match(')')) {
                printf("name: %.*s, x: %d, y: %d\n", static_cast<int>(tok.size()), tok.data(), x, y);
            }
        } else {
            p.Next();
        }
    }

    // name: point, x: 1, y: 20
    // name: vector, x: -2, y: -30

    return 0;
}
```

## Example: math expression

This example shows how to parse a math expression.

```cpp
#include <cstdio>
#include <functional>
#include "parser.hpp"

int main()
{
    Parser p("(2+3)*4");

    std::function<bool(int*)> expr, term, fact;

    expr = [&](int* out) {
        if (term(out)) {
            int r;
            if (p.Match('+') && expr(&r)) {
                *out += r;
            } else if (p.Match('-') && expr(&r)) {
                *out -= r;
            }
            return true;
        }
        return false;
    };
    term = [&](int* out) {
        if (fact(out)) {
            int r;
            if (p.Match('*') && term(&r)) {
                *out *= r;
            } else if (p.Match('/') && term(&r)) {
                *out /= r;
            }
            return true;
        }
        return false;
    };
    fact = [&](int* out) {
        return (p.Match('(') && expr(out) && p.Match(')')) || p.NumberOut(out);
    };

    int result = 0;
    expr(&result);
    printf("Result: %d\n", result);
    // Result: 20

    return 0;
}
```
