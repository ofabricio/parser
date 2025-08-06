#ifndef OFAB_PARSER_HPP
#define OFAB_PARSER_HPP

#include <cstdlib>
#include <string>

// Text parser.
class Parser {
public:
    Parser(const std::string_view text)
        : text(text) { };

    // Convenience function that allows to look ahead.
    // The parser goes back to the mark m on cond either true or false.
    bool Peek(std::string_view m, bool cond);
    // Convenience function that undoes the operation if cond is false,
    // rewinding the parser to the marked position m.
    // Useful for recovering from operations that may fail mid-way.
    bool Undo(std::string_view m, bool cond);
    // Convenience function that outputs the token from
    // the mark m to the current position if cond is true.
    bool Out(std::string_view m, bool cond, std::string_view& out);
    bool Out(std::string_view m, bool cond, std::string& out);
    bool Out(std::string_view m, bool cond, std::vector<std::string_view>& out);
    bool Out(std::string_view m, bool cond, std::vector<std::string>& out);
    // Matches a number and outputs it.
    // Advances the parser if it matches.
    bool Number(int& out);
    // Matches a number.
    // Advances the parser if it matches.
    bool Number();
    // Matches a string enclosed in quotes. Skips escaped quotes.
    // Advances the parser if it matches.
    bool String(char quote);
    // Matches a line (up to a newline character).
    // Advances the parser if it matches.
    bool Line();
    // Matches whitespace characters.
    // Advances the parser if it matches.
    bool Space();
    // Matches any character that is not the string.
    // Advances the parser by one character if it does not match.
    bool Not(std::string_view);
    // Matches any character that is not in the given range.
    // Advances the parser by one character if it does not match.
    bool Not(std::pair<char, char> range);
    // Matches any character that is not any given one.
    // Advances the parser by one character if it does not match.
    template <typename... Char>
    bool Not(Char... any);
    // Matches any character.
    // Advances the parser if it matches.
    bool Any();
    // Matches until any given character range.
    // Advances the parser if it matches.
    bool Until(std::pair<char, char> range);
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
    // Matches any given character range.
    // Advances the parser if it matches.
    bool Match(std::pair<char, char> range);
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
    // Tests any given character range.
    bool Equal(std::pair<char, char>);
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
    // Advances the parser by one characters.
    void Next();
    // Advances the parser by n characters.
    void Advance(int);
    // Tells if there are more characters to parse.
    bool More();

private:
    std::string_view text;
};

bool Parser::Out(std::string_view m, bool cond, std::string_view& out)
{
    if (cond) {
        out = Token(m);
    }
    return cond;
}

bool Parser::Out(std::string_view m, bool cond, std::string& out)
{
    if (cond) {
        out = Token(m);
    }
    return cond;
}

bool Parser::Out(std::string_view m, bool cond, std::vector<std::string_view>& out)
{
    if (cond) {
        out.push_back(Token(m));
    }
    return cond;
}

bool Parser::Out(std::string_view m, bool cond, std::vector<std::string>& out)
{
    if (cond) {
        out.push_back(std::string(Token(m)));
    }
    return cond;
}

bool Parser::Peek(std::string_view m, bool cond)
{
    Back(m);
    return cond;
}

bool Parser::Undo(std::string_view m, bool cond)
{
    if (!cond) {
        Back(m);
    }
    return cond;
}

bool Parser::Number(int& out)
{
    auto m = Mark();
    if (Number()) {
        out = atoi(m.data());
        return true;
    }
    return false;
}

bool Parser::Number()
{
    return Undo(Mark(), (Match('-', '+') || true) && While({ '0', '9' }));
}

bool Parser::String(char quote)
{
    auto m = Mark();
    if (Match(quote)) {
        while (Not(quote, '\\') || (Match('\\') && Any())) { }
    }
    return Undo(m, Match(quote));
}

bool Parser::Line()
{
    return Until('\n') + Match('\n');
}

bool Parser::Space()
{
    return While({ '\0' + 1, ' ' });
}

bool Parser::Not(std::string_view v)
{
    return !Equal(v) && Any();
}

bool Parser::Not(std::pair<char, char> range)
{
    return !Equal(range) && Any();
}

template <typename... Char>
bool Parser::Not(Char... args)
{
    return !Equal(args...) && Any();
}

bool Parser::Until(std::pair<char, char> range)
{
    auto m = Mark();
    while (Not(range)) { }
    return Moved(m);
}

template <typename... Char>
bool Parser::Until(Char... any)
{
    auto m = Mark();
    while (Not(any...)) { }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a)
{
    auto m = Mark();
    while (Match(a)) { }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b)
{
    auto m = Mark();
    while ((Match(a) || Match(b))) { }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b, std::pair<char, char> c)
{
    auto m = Mark();
    while ((Match(a) || Match(b) || Match(c))) { }
    return Moved(m);
}

bool Parser::While(std::pair<char, char> a, std::pair<char, char> b, std::pair<char, char> c, std::pair<char, char> d)
{
    auto m = Mark();
    while ((Match(a) || Match(b) || Match(c) || Match(d))) { }
    return Moved(m);
}

bool Parser::Match(std::pair<char, char> range)
{
    return Equal(range) && Any();
}

template <typename... Char>
bool Parser::Match(char v, Char... any)
{
    return Match(v) || Match(any...);
}

bool Parser::Match(char v)
{
    return Equal(v) && Any();
}

bool Parser::Match(std::string_view v)
{
    if (Equal(v)) {
        Advance(v.size());
        return true;
    }
    return false;
}

bool Parser::Any()
{
    if (More()) {
        Next();
        return true;
    }
    return false;
}

bool Parser::Equal(std::pair<char, char> range)
{
    return Curr() >= range.first && Curr() <= range.second;
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
