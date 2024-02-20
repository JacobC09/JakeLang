#include "parser.h"
#include "variant"
#include "util.h"

Parser::Parser(std::string source) : scanner(source) {
    hadError = false;
}

Ast Parser::parse() {
    Ast ast;

    advance();
    while (!isFinished()) {
        ast.body.push_back(statement());
    }

    return std::move(ast);  
}

void Parser::advance() {
    if (hadError) return;

    prev = cur;
    cur = scanner.nextToken();

    if (cur.type == TokenType::Error) {
        errorAt(cur, formatStr("Invalid Token: %s", std::string(cur.value)));
    }
}

void Parser::consume(TokenType type, std::string msg) {
    if (cur.type == type) {
        advance();
        return;
    }

    errorAt(cur, msg);
};

bool Parser::check(TokenType type) {
    return cur.type == type;
}

void Parser::errorAt(Token& token, std::string msg) {
    hadError = true;

    printf("Error at line %d:", token.line);
    printf("\t%s", msg.c_str());
}

bool Parser::isFinished() {
    return check(TokenType::EndOfFile) || hadError;
}

ParseRule Parser::getRule(TokenType type) {
    switch (type) {
        case TokenType::Number: return ParseRule {&Parser::number, NULL, Precedence::None};
        case TokenType::Identifier: return ParseRule {&Parser::identifier, NULL, Precedence::None};
        case TokenType::String: return ParseRule {&Parser::string, NULL, Precedence::None};
        case TokenType::True: return ParseRule {&Parser::literal, NULL, Precedence::None};
        case TokenType::False: return ParseRule {&Parser::literal, NULL, Precedence::None};
        case TokenType::None: return ParseRule {&Parser::literal, NULL, Precedence::None};
        case TokenType::Bang: return ParseRule {&Parser::unary, NULL, Precedence::None};
        case TokenType::Minus: return ParseRule {&Parser::unary, &Parser::binary, Precedence::Term};
        case TokenType::Plus: return ParseRule {NULL, &Parser::binary, Precedence::Term};
        case TokenType::Asterisk: return ParseRule {NULL, &Parser::binary, Precedence::Factor};
        case TokenType::Slash: return ParseRule {NULL, &Parser::binary, Precedence::Factor};

        case TokenType::EqualEqual:
        case TokenType::BangEqual: 
            return ParseRule {NULL, &Parser::binary, Precedence::Equality};

        case TokenType::Greater:
        case TokenType::Less:
        case TokenType::GreaterEqual:
        case TokenType::LessEqual: 
            return ParseRule {NULL, &Parser::binary, Precedence::Comparison};

        default:
            return ParseRule {NULL, NULL, Precedence::None};
    }

}

Expr Parser::number(bool isLvalue) {
    if (prev.value.front() == '.')
        return NumLiteral {std::stod("0." + std::string(prev.value))};

    return NumLiteral {std::stod(std::string(prev.value))};
}

Expr Parser::identifier(bool isLvalue) {
    return Identifier {std::string(prev.value)};
}

Expr Parser::string(bool isLvalue) {

    /* Todo: Add formated strings */

    return StrLiteral {std::string(prev.value.data() + 1, prev.value.size() - 2)};
}

Expr Parser::literal(bool isLvalue) {
    switch (prev.type) {
        case TokenType::True:
            return BoolLiteral {true};
        case TokenType::False:
            return BoolLiteral {false};
        case TokenType::None:
            return NoneLiteral {};
        default: 
            return Empty {};
    }
}

Expr Parser::grouping(bool isLvalue) {
    advance();
    Expr expr = expression();
    consume(TokenType::RightParen, "Expected closing parenthesis: ')'");
    return expr;
}

Expr Parser::unary(bool isLvalue) {
    
}

Expr Parser::binary(bool isLvalue) {

}

Expr Parser::parsePrecedence(Precedence precedence) {
    advance();

    ParseRule rule = getRule(prev.type);
    if (rule.prefix == NULL) {
        errorAt(prev, "Expected an expression");
    }

    bool isLvalue = precedence <= Precedence::Assignment;
    (this->*rule.prefix)(isLvalue);

    while ((rule = getRule(cur.type)).precedence >= precedence) {
        advance();
        (this->*rule.infix)(isLvalue);
    }

    bool isAssignment = check(TokenType::Equal) || check(TokenType::PlusEqual) || check(TokenType::MinusEqual) ||
        check(TokenType::AsteriskEqual) || check(TokenType::SlashEqual);

    if (isLvalue && isAssignment) {
        errorAt(prev, "Invalid assignment target");
        advance();
    }
}

Expr Parser::expression() {
    return parsePrecedence(Precedence::Assignment);
}

Stmt Parser::expressionStatement() {
    Stmt stmt = ExprStmt {expression()};
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return stmt;
}

Stmt Parser::statement() {
    switch (cur.type) {
        default:
            return expressionStatement();
    }
}
