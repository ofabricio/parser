#ifndef OFAB_PARSER_HPP
#define OFAB_PARSER_HPP

#include <cstdlib>
#include <string>

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
    // Matches until the given character.
    // Advances the parser if it matches.
    bool Until(char);
    // Matches the given character range.
    // Advances the parser if it matches.
    bool While(char min, char max);
    // Matches the given character.
    // Advances the parser if it matches.
    bool Match(char);
    // Matches the given string.
    // Advances the parser if it matches.
    bool Match(std::string_view);
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
    if (While('0', '9')) {
        return true;
    }
    Back(m);
    return false;
}

bool Parser::Line()
{
    auto m = Mark();
    while (More() && !Match('\n')) {
        Next();
    }
    return Moved(m);
}

bool Parser::Space()
{
    return While('\0' + 1, ' ');
}

bool Parser::Until(char v)
{
    auto m = text;
    while (More() && !Equal(v)) {
        Next();
    }
    return Moved(m);
}

bool Parser::While(char min, char max)
{
    auto m = text;
    while (Curr() >= min && Curr() <= max) {
        Next();
    }
    return Moved(m);
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
