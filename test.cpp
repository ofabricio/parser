#include <assert.h>
#include <functional>

#include "parser.hpp"

void Example_Expr()
{
    Parser p("(6-1)*4*2+(1+3)*(16/2)");

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
    assert(expr(&result) == true);
    assert(result == 72);
}

void Example()
{
    Parser p("point(1 20)\n"
             "vector(-2 -30)");

    std::vector<std::tuple<std::string_view, int, int>> results;
    while (p.More()) {
        auto m = p.Mark();
        if (p.While('a', 'z')) {
            auto tok = p.Token(m);
            int x, y;
            if (p.Match('(') && p.NumberOut(&x) && p.Space() && p.NumberOut(&y) && p.Match(')')) {
                results.emplace_back(tok, x, y);
            }
        } else {
            p.Next();
        }
    }

    assert(results.size() == 2);
    assert(results[0] == std::make_tuple("point", 1, 20));
    assert(results[1] == std::make_tuple("vector", -2, -30));
}

void TestOut()
{
    Parser p("aaa111bbb");
    std::string_view out;
    assert(p.Out(p.Mark(), p.Number(), &out) == false);
    assert(out == "");
    assert(p.Out(p.Mark(), p.While('a', 'z'), &out) == true);
    assert(out == "aaa");
    assert(p.Out(p.Mark(), p.Number(), &out) == true);
    assert(out == "111");
    assert(p.Out(p.Mark(), p.While('a', 'z'), &out) == true);
    assert(out == "bbb");
}

void TestNumberOut_Int()
{
    int out;

    Parser p("2");
    assert(p.NumberOut(&out) == true);
    assert(out == 2);

    p = Parser("23");
    assert(p.NumberOut(&out) == true);
    assert(out == 23);

    p = Parser("-2");
    assert(p.NumberOut(&out) == true);
    assert(out == -2);

    p = Parser("+2");
    assert(p.NumberOut(&out) == true);
    assert(out == 2);

    p = Parser("x");
    assert(p.NumberOut(&out) == false);
}

void TestNumber()
{
    Parser p("2");
    assert(p.Number() == true);
    assert(p.Tail() == "");

    p = Parser("23");
    assert(p.Number() == true);
    assert(p.Tail() == "");

    p = Parser("-2");
    assert(p.Number() == true);
    assert(p.Tail() == "");

    p = Parser("+2");
    assert(p.Number() == true);
    assert(p.Tail() == "");

    p = Parser("x");
    assert(p.Number() == false);
    assert(p.Tail() == "x");
}

void TestLine()
{
    Parser p("\n");
    assert(p.Line() == true);
    assert(p.Tail() == "");

    p = Parser("a\n");
    assert(p.Line() == true);
    assert(p.Tail() == "");

    p = Parser("aaa\n");
    assert(p.Line() == true);
    assert(p.Tail() == "");

    p = Parser("");
    assert(p.Line() == false);
    assert(p.Tail() == "");

    p = Parser("abc");
    assert(p.Line() == true);
    assert(p.Tail() == "");
}

void TestSpace()
{
    Parser p("   x");
    assert(p.Space() == true);
    assert(p.Tail() == "x");
}

void TestUntil()
{
    Parser p("\n");
    assert(p.Until('\n') == false);
    assert(p.Tail() == "\n");

    p = Parser("a\n");
    assert(p.Until('\n') == true);
    assert(p.Tail() == "\n");

    p = Parser("aaa\n");
    assert(p.Until('\n') == true);
    assert(p.Tail() == "\n");

    p = Parser("");
    assert(p.Until('\n') == false);
    assert(p.Tail() == "");

    p = Parser("abc");
    assert(p.Until('\n') == true);
    assert(p.Tail() == "");
}

void TestWhile()
{
    Parser p("amz1");
    assert(p.While('a', 'z') == true);
    assert(p.Tail() == "1");
}

void TestMatch_Str()
{
    Parser p("Hi");
    assert(p.Match("No") == false);
    assert(p.Tail() == "Hi");
    assert(p.Match("Hi") == true);
    assert(p.Tail() == "");
}

void TestMatch_Char()
{
    Parser p("Hi");
    assert(p.Match('X') == false);
    assert(p.Match('H') == true);
    assert(p.Tail() == "i");
    assert(p.Match('i') == true);
    assert(p.Tail() == "");
}

void TestEqual_Str()
{
    Parser p("Hi");
    assert(p.Equal("Hi") == true);
    assert(p.Equal("No") == false);
    assert(p.Tail() == "Hi");
}

void TestEqual_Char()
{
    Parser p("Hi");
    assert(p.Equal('H') == true);
    assert(p.Equal('i') == false);
    assert(p.Tail() == "Hi");
}

void TestBack()
{
    Parser p("Hi");
    auto m = p.Mark();
    p.Advance(2);
    p.Back(m);
    assert(p.Tail() == "Hi");
}

void TestToken()
{
    Parser p("HelloWorld");
    auto m = p.Mark();
    p.Advance(5);
    assert(p.Token(m) == "Hello");
}

void TestTail()
{
    Parser p("Hi");
    assert(p.Tail() == "Hi");
}

void TestNext()
{
    Parser p("Hi");
    p.Next();
    assert(p.Tail() == "i");
    p.Next();
    assert(p.Tail() == "");
}

void TestCurr()
{
    Parser a("A");
    assert(a.Curr() == 'A');
    Parser b("");
    assert(b.Curr() == '\0');
}

void TestMore()
{
    Parser a("A");
    assert(a.More() == true);
    Parser b("");
    assert(b.More() == false);
}

int main()
{
    Example_Expr();
    Example();
    TestOut();
    TestNumberOut_Int();
    TestNumber();
    TestLine();
    TestSpace();
    TestUntil();
    TestWhile();
    TestMatch_Str();
    TestMatch_Char();
    TestEqual_Str();
    TestEqual_Char();
    TestBack();
    TestToken();
    TestTail();
    TestNext();
    TestCurr();
    TestMore();
    return 0;
}
