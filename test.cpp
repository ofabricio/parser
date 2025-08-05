#include <assert.h>
#include <functional>

#include "parser.hpp"

void Example_Expr()
{
    Parser p("(6-1)*4*2+(1+3)*(16/2)");

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
    assert(expr(result) == true);
    assert(result == 72);
}

void Example_Json()
{
    Parser p(R"({ "name": "John", "country": [ "USA", "BRAZIL" ] })");

    std::function<bool(std::string&)> jsn, obj, arr, str, key;

    jsn = [&](std::string& out) {
        p.Space();
        return obj(out) || arr(out) || str(out);
    };
    obj = [&](std::string& out) {
        if (p.Match('{')) {
            if (!key(out)) {
                return p.Match('}');
            }
            while ((p.Space() || true) && p.Undo(p.Mark(), p.Match(',') && key(out))) { }
            return p.Match('}');
        }
        return false;
    };
    arr = [&](std::string& out) {
        if (p.Match('[')) {
            if (!jsn(out)) {
                return p.Match(']');
            }
            while ((p.Space() || true) && p.Undo(p.Mark(), p.Match(',') && jsn(out))) { }
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
    assert(jsn(out) == true);
    assert(out == R"("John"; "USA"; "BRAZIL"; )");
}

void Example()
{
    Parser p("point(1 20)\n"
             "vector(-2 -30)");

    std::vector<std::tuple<std::string_view, int, int>> results;
    while (p.More()) {
        auto m = p.Mark();
        if (p.While({ 'a', 'z' })) {
            auto tok = p.Token(m);
            int x, y;
            if (p.Match('(') && p.Number(x) && p.Space() && p.Number(y) && p.Match(')')) {
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

void TestString()
{
    Parser p(R"("")");
    assert(p.String('"') == true);
    assert(p.Tail() == "");

    p = Parser(R"("a")");
    assert(p.String('"') == true);
    assert(p.Tail() == "");

    p = Parser(R"("a\"b\"c")");
    assert(p.String('"') == true);
    assert(p.Tail() == "");

    p = Parser(R"("a\nb\"c")");
    assert(p.String('"') == true);
    assert(p.Tail() == "");

    p = Parser(R"("a)");
    assert(p.String('"') == false);
    assert(p.Tail() == R"("a)");

    p = Parser("''");
    assert(p.String('\'') == true);
    assert(p.Tail() == "");

    p = Parser("'a'");
    assert(p.String('\'') == true);
    assert(p.Tail() == "");

    p = Parser("'a\\'b\\'c'");
    assert(p.String('\'') == true);
    assert(p.Tail() == "");

    p = Parser("'a\\'b\\'c'");
    assert(p.String('\'') == true);
    assert(p.Tail() == "");

    p = Parser("'a");
    assert(p.String('\'') == false);
    assert(p.Tail() == "'a");
}

void TestPeek()
{
    Parser p("1+2");
    assert(p.Peek(p.Mark(), p.Match('1') && p.Match('+') && p.Match('3')) == false);
    assert(p.Tail() == "1+2");
    assert(p.Peek(p.Mark(), p.Match('1') && p.Match('+') && p.Match('2')) == true);
    assert(p.Tail() == "1+2");
}

void TestUndo()
{
    Parser p("1+2");
    assert(p.Undo(p.Mark(), p.Match('1') && p.Match('+') && p.Match('3')) == false);
    assert(p.Tail() == "1+2");
    assert(p.Undo(p.Mark(), p.Match('1') && p.Match('+') && p.Match('2')) == true);
    assert(p.Tail() == "");
}

void TestOut()
{
    Parser p("123a");

    p = Parser("123a");
    std::string_view out0;
    assert(p.Out(p.Mark(), p.Match('a'), out0) == false);
    assert(out0 == "");

    p = Parser("123a");
    std::string_view out1;
    assert(p.Out(p.Mark(), p.Number(), out1) == true);
    assert(out1 == "123");

    p = Parser("123a");
    std::string out2;
    assert(p.Out(p.Mark(), p.Number(), out2) == true);
    assert(out2 == "123");

    p = Parser("111a222");
    std::vector<std::string> out3;
    assert(p.Out(p.Mark(), p.Number(), out3) == true);
    p.Match('a');
    assert(p.Out(p.Mark(), p.Number(), out3) == true);
    assert(out3 == (std::vector<std::string> { "111", "222" }));

    p = Parser("111a222");
    std::vector<std::string_view> out4;
    assert(p.Out(p.Mark(), p.Number(), out4) == true);
    p.Match('a');
    assert(p.Out(p.Mark(), p.Number(), out4) == true);
    assert(out4 == (std::vector<std::string_view> { "111", "222" }));
}

void TestNumber_Int()
{
    int out;

    Parser p("2");
    assert(p.Number(out) == true);
    assert(out == 2);

    p = Parser("23");
    assert(p.Number(out) == true);
    assert(out == 23);

    p = Parser("-2");
    assert(p.Number(out) == true);
    assert(out == -2);

    p = Parser("+2");
    assert(p.Number(out) == true);
    assert(out == 2);

    p = Parser("-02");
    assert(p.Number(out) == true);
    assert(out == -2);

    p = Parser("x");
    assert(p.Number(out) == false);
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

    p = Parser("-");
    assert(p.Number() == false);
    assert(p.Tail() == "-");

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

void TestNot()
{
    Parser p("ab");
    assert(p.Not('b') == true);
    assert(p.Tail() == "b");
    assert(p.Not('b') == false);
    assert(p.Tail() == "b");

    p = Parser("ab");
    assert(p.Not('b', 'c') == true);
    assert(p.Tail() == "b");
    assert(p.Not('a', 'b') == false);
    assert(p.Tail() == "b");

    p = Parser("ab");
    assert(p.Not({ '0', '9' }) == true);
    assert(p.Tail() == "b");
    assert(p.Not({ 'a', 'z' }) == false);
    assert(p.Tail() == "b");

    p = Parser("aNot");
    assert(p.Not("Not") == true);
    assert(p.Tail() == "Not");
    assert(p.Not("Not") == false);
    assert(p.Tail() == "Not");

    p = Parser("");
    assert(p.Not('x') == false);
}

void TestAny()
{
    Parser p("ab");
    assert(p.Any() == true);
    assert(p.Tail() == "b");
    assert(p.Any() == true);
    assert(p.Tail() == "");
    assert(p.Any() == false);
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

    p = Parser("ab.");
    assert(p.Until('.', ',') == true);
    assert(p.Tail() == ".");

    p = Parser("ab.");
    assert(p.Until(',', '.') == true);
    assert(p.Tail() == ".");

    p = Parser("ab;");
    assert(p.Until(',', '.') == true);
    assert(p.Tail() == "");

    p = Parser("");
    assert(p.Until(',', '.') == false);
    assert(p.Tail() == "");
}

void TestUntil_Range()
{
    Parser p("abc123");
    assert(p.Until({ '0', '9' }) == true);
    assert(p.Tail() == "123");
}

void TestWhile()
{
    Parser p("Name_123()");
    assert(p.While({ 'A', 'Z' }) == true);
    assert(p.Tail() == "ame_123()");

    p = Parser("Name_123()");
    assert(p.While({ 'A', 'Z' }, { 'a', 'z' }) == true);
    assert(p.Tail() == "_123()");

    p = Parser("Name_123()");
    assert(p.While({ 'A', 'Z' }, { 'a', 'z' }, { '_', '_' }) == true);
    assert(p.Tail() == "123()");

    p = Parser("Name_123()");
    assert(p.While({ 'A', 'Z' }, { 'a', 'z' }, { '_', '_' }, { '0', '9' }) == true);
    assert(p.Tail() == "()");
}

void TestMatch_Range()
{
    Parser p("abc");
    assert(p.Match({ 'A', 'C' }) == false);
    assert(p.Match({ 'a', 'c' }) == true);
    assert(p.Tail() == "bc");
    assert(p.Match({ 'a', 'c' }) == true);
    assert(p.Tail() == "c");
    assert(p.Match({ 'a', 'c' }) == true);
    assert(p.Tail() == "");
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

    p = Parser("Hi");
    assert(p.Match('X', 'H') == true);
    assert(p.Tail() == "i");

    p = Parser("Hi");
    assert(p.Match('H', 'X') == true);
    assert(p.Tail() == "i");

    p = Parser("Hi");
    assert(p.Match('X', 'Y') == false);
    assert(p.Tail() == "Hi");
}

void TestEqual_Str()
{
    Parser p("Hi");
    assert(p.Equal("Hi") == true);
    assert(p.Equal("No") == false);
    assert(p.Tail() == "Hi");
}

void TestEqual_Range()
{
    Parser p("a");
    assert(p.Equal({ 'A', 'B' }) == false);
    assert(p.Equal({ 'a', 'a' }) == true);
    assert(p.Equal({ 'a', 'b' }) == true);
    assert(p.Tail() == "a");
}

void TestEqual_Char()
{
    Parser p("Hi");
    assert(p.Equal('H') == true);
    assert(p.Equal('i') == false);
    assert(p.Tail() == "Hi");

    assert(p.Equal('H', 'X') == true);
    assert(p.Equal('X', 'H') == true);
    assert(p.Equal('X', 'Y') == false);
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
    Example_Json();
    Example();
    TestString();
    TestPeek();
    TestUndo();
    TestOut();
    TestNumber_Int();
    TestNumber();
    TestLine();
    TestSpace();
    TestNot();
    TestAny();
    TestUntil_Range();
    TestUntil();
    TestWhile();
    TestMatch_Range();
    TestMatch_Str();
    TestMatch_Char();
    TestEqual_Range();
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
