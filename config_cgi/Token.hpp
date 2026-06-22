#pragma once

#include <string>

enum TokenType {
    WORD,
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON
};

class Token{
public:
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v);
};