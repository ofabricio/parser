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
            if (p.Match('(') && p.Number(x) && p.Space() && p.Number(y) && p.Match(')')) {
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
Note that this example is not production-ready.

```cpp
#include <cstdio>
#include <functional>
#include "parser.hpp"

int main()
{
    Parser p("(2+3)*4");

    std::function<bool(int&)> expr, term, fact;

    expr = [&](int& out) {
        if (term(out)) {
            int r;
            if (p.Match('+') && expr(r)) {
                out += r;
            } else if (p.Match('-') && expr(r)) {
                out -= r;
            }
            return true;
        }
        return false;
    };
    term = [&](int& out) {
        if (fact(out)) {
            int r;
            if (p.Match('*') && term(r)) {
                out *= r;
            } else if (p.Match('/') && term(r)) {
                out /= r;
            }
            return true;
        }
        return false;
    };
    fact = [&](int& out) {
        return (p.Match('(') && expr(out) && p.Match(')')) || p.Number(out);
    };

    int result = 0;
    expr(result);
    printf("Result: %d\n", result);
    // Result: 20

    return 0;
}
```

## Example: json

This example shows how to parse a Json and get all string values.
Note that this example is not production-ready.

```cpp
#include <cstdio>
#include <functional>
#include "parser.hpp"

int main()
{
    Parser p(R"({ "name": "John", "country": [ "USA", "BRAZIL" ] })");

    std::function<bool(std::string&)> jsn, obj, arr, str, key;

    jsn = [&](std::string& out) {
        p.Space();
        return obj(out) || arr(out) || str(out);
    };
    obj = [&](std::string& out) {
        if (p.Match('{')) {
            if (!key(out)) return p.Match('}');
            while ((p.Space() || true) && p.Undo(p.Mark(), p.Match(',') && key(out)));
            return p.Match('}');
        }
        return false;
    };
    arr = [&](std::string& out) {
        if (p.Match('[')) {
            if (!jsn(out)) return p.Match(']');
            while ((p.Space() || true) && p.Undo(p.Mark(), p.Match(',') && jsn(out)));
            return p.Match(']');
        }
        return false;
    };
    str = [&](std::string& out) {
        auto m = p.Mark();
        if (p.String('"')) {
            out += std::string(p.Token(m)) + "; ";
            return true;
        }
        return false;
    };
    key = [&](std::string& out) {
        p.Space();
        std::string k;
        return str(k) && (p.Space() || true) && p.Match(':') && jsn(out);
    };

    std::string out;
    jsn(out);
    printf("%s\n", out.c_str());
    // "John"; "USA"; "BRAZIL";

    return 0;
}
```

## Introduction

This library implements a **Mark, Match and Move** mechanism,
which is a simple, intuitive way to parse and collect tokens.

All matching operations are based on this pattern:

```cpp
auto m = p.Mark();
if (p.Match("something")) {
    auto t = p.Token(m);
}
```

First a mark is set to the current position.
Then the parser advances on a match.
Finally, the mark is used to extract the matched token on success.
It could be used to move the parser back to the marked position if needed.

That's all about it.
