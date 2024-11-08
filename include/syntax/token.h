#pragma once
#include <string>

struct SourceView {
    int index, length, line, column;
};

SourceView operator | (const SourceView& left, const SourceView& right);

enum class TokenType {
    // Single Char
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Dot,
    Plus,
    Minus,
    Slash,
    Asterisk,
    Carret,
    Semicolon,
    Percent,

    // One or Two Char
    Bang,
    BangEqual,
    Equal,
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    PlusEqual,
    MinusEqual,
    AsteriskEqual,
    SlashEqual,
    CarretEqual,

    // Literals
    Identifier,
    String,
    Number,
    True,
    False,
    None,

    // Keywords
    Print,
    If,
    Else,
    Loop,
    While,
    For,
    In,
    Continue,
    Break,
    Return,
    Func,
    Var,
    Exit,
    And,
    Or,
    Type,

    Error,
    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
    SourceView view;
};
