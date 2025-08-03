#ifndef OFAB_PARSER_HPP
#define OFAB_PARSER_HPP

#include <cstdlib>
#include <string>

#define InRange(ch, range) (ch >= range.first && ch <= range.second)

// Text parser.
class Parser {
public:
    Parser(const std::string_view text)
        : text(text) { };

    // This is a convenience function that outputs the token from
    // the mark m to the current position if cond matches.
    bool Out(std::string_view m, bool cond, std::string_view* out);
    // Matches a number and outputs it.
    // Advances the parser if it matches.
    bool NumberOut(int* out);
    // Matches a number.
    // Advances the parser if it matches.
    bool Number();
    // Matches a line (up to a newline character).
    // Advances the parser if it matches.
    bool Line();
    // Matches whitespace characters.
    // Advances the parser if it matches.
    bool Space();
    // Matches until any given character.
    // Advances the parser if it matches.
    template <typename... Char>
    bool Until(Char... any);
    // Matches while in any given character range.
    // Advances the parser if it matches.
    bool While(std::pair<char, char>);
    bool While(std::pair<char, char>, std::pair<char, char>);
    bool While(std::pair<char, char>, std::pair<char, char>, std::pair<char, char>);
    bool While(std::pair<char, char>, std::pair<char, char>, std::pair<char, char>, std::pair<char, char>);
    // Matches any given character.
    // Advances the parser if it matches.
    template <typename... Char>
    bool Match(char, Char...);
    // Matches the given character.
    // Advances the parser if it matches.
    bool Match(char);
    // Matches the given string.
    // Advances the parser if it matches.
    bool Match(std::string_view);
    // Tests any given character.
    template <typename... Char>
    bool Equal(char, Char...);
    // Tests the given character.
    bool Equal(char);
    // Tests the given string.
    bool Equal(std::string_view);
    // Returns a mark to the current position.
    std::string_view Mark();
    // Sets the parser to the marked position.
    void Back(std::string_view m);
    // Tells if the parser has moved from the marked position.
    bool Moved(std::string_view m);
    // Returns the token from the marked position to the current position.
    std::string_view Token(std::string_view m);
    // Returns the remaining text.
    std::string_view Tail();
    // Returns the current character.
    char Curr();
    // Advances the parser by 1 character.
    void Next();
    // Advances the parser by n characters.
    void Advance(int);
    // Tells if there are more characters to parse.
    bool More();

private:
    std::string_view text;
};

bool Parser::Out(std::string_view m, bool cond, std::string_view* out)
{
    if (cond) {
        *out = Token(m);
        return true;
    }
    Back(m);
    return false;
}

bool Parser::NumberOut(int* out)
{
    auto m = Mark();
    if (Number()) {
        *out = atoi(m.data());
        return true;
    }
    return false;
}

bool Parser::Number()
{
    auto m = Mark();
    Match('-') || Match('+');
    if (While({ '0', '9' })) {
        return true;
    }
    Back(m);
    return false;
}

bool Parser::Line()
{
    return Until('\n') + Match('\n');
}

bool Parser::Space()
{
    return While({ '\0' + 1, ' ' });
}

template <typename... Char>
bool Parser::Until(Char... any)
{
    auto m = Mark();
    while (More() && !Equal(any...)) {
        Next();
    }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a)
{
    auto m = Mark();
    while (More() && InRange(Curr(), a)) {
        Next();
    }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b)
{
    auto m = Mark();
    while (More() && (InRange(Curr(), a) || InRange(Curr(), b))) {
        Next();
    }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b, std::pair<char, char> c)
{
    auto m = Mark();
    while (More() && (InRange(Curr(), a) || InRange(Curr(), b) || InRange(Curr(), c))) {
        Next();
    }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b, std::pair<char, char> c, std::pair<char, char> d)
{
    auto m = Mark();
    while (More() && (InRange(Curr(), a) || InRange(Curr(), b) || InRange(Curr(), c) || InRange(Curr(), d))) {
        Next();
    }
    return Moved(m);
}

template <typename... Char>
bool Parser::Match(char v, Char... any)
{
    return Match(v) || Match(any...);
}

bool Parser::Match(char v)
{
    if (Equal(v)) {
        Next();
        return true;
    }
    return false;
}

bool Parser::Match(std::string_view v)
{
    if (Equal(v)) {
        Advance(v.size());
        return true;
    }
    return false;
}

template <typename... Char>
bool Parser::Equal(char v, Char... any)
{
    return Equal(v) || Equal(any...);
}

bool Parser::Equal(char v)
{
    return Curr() == v;
}

bool Parser::Equal(std::string_view v)
{
    return text.substr(0, v.size()) == v;
}

std::string_view Parser::Mark()
{
    return text;
}

void Parser::Back(std::string_view m)
{
    text = m;
}

bool Parser::Moved(std::string_view m)
{
    return m.size() != text.size();
}

std::string_view Parser::Token(std::string_view m)
{
    return m.substr(0, m.size() - text.size());
}

std::string_view Parser::Tail()
{
    return text;
}

char Parser::Curr()
{
    return text.front();
}

void Parser::Next()
{
    text.remove_prefix(1);
}

void Parser::Advance(int n)
{
    text.remove_prefix(n);
}

bool Parser::More()
{
    return !text.empty();
}

#endif
