#include "parser.h"
#include "util.h"

Parser::Parser(std::string source) {
    scanner = Scanner(source);
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

bool Parser::match(TokenType type) {
    if (isFinished()) return false;
    if (check(type)) return false;

    advance();
    return true; 
}

template<typename... Args>
bool Parser::match(TokenType type, Args... args) {
    return match(type) || match(args...);
}

void Parser::errorAt(Token& token, std::string msg) {
    hadError = true;

    printf("Error at line %d:", token.line);
    printf("\t%s", msg.c_str());
}

bool Parser::isFinished() {
    return check(TokenType::EndOfFile) || hadError;
}

Expr Parser::expression() {
    return equality();
}

Expr Parser::equality() {
    Expr expr = comparison();

    while (match(TokenType::EqualEqual, TokenType::BangEqual)) {
        BinaryExpr::Operation op;
        switch (prev.type) {
            case TokenType::EqualEqual:
                op = BinaryExpr::Operation::Equal;
                break;
            case TokenType::BangEqual:
                op = BinaryExpr::Operation::NotEqual;
                break;
            default: break;
        }
        
        expr = BinaryExpr {op, expr, comparison()};
    }

    return expr;
}

Expr Parser::comparison() {
    Expr expr = term();

    while (match(TokenType::Greater, TokenType::Less, TokenType::LessEqual, TokenType::GreaterEqual)) {
        BinaryExpr::Operation op;
        switch (prev.type) {
            case TokenType::Greater:
                op = BinaryExpr::Operation::GreaterThan;
                break;
            case TokenType::Less:
                op = BinaryExpr::Operation::LessThan;
                break;
            case TokenType::LessEqual:
                op = BinaryExpr::Operation::LessThanOrEq;
                break;
            case TokenType::GreaterEqual:
                op = BinaryExpr::Operation::GreaterThanOrEq;
                break;
            default: break;
        }
        
        expr = BinaryExpr {op, expr, term()};
    }

    return expr;
}

Expr Parser::term() {
    Expr expr = factor();

    while (match(TokenType::Plus, TokenType::Minus)) {
        BinaryExpr::Operation op;
        switch (prev.type) {
            case TokenType::Plus:
                op = BinaryExpr::Operation::Add;
                break;
            case TokenType::Minus:
                op = BinaryExpr::Operation::Subtract;
                break;
            default: break;
        }
        
        expr = BinaryExpr {op, expr, factor()};
    }

    return expr;
}

Expr Parser::factor() {
    Expr expr = unary();

    while (match(TokenType::Asterisk, TokenType::Slash)) {
        BinaryExpr::Operation op;
        switch (prev.type) {
            case TokenType::Asterisk:
                op = BinaryExpr::Operation::Multiply;
                break;
            case TokenType::Slash:
                op = BinaryExpr::Operation::Divide;
                break;
            default: break;
        }
        
        expr = BinaryExpr {op, expr, unary()};
    }

    return expr;
}

Expr Parser::unary() {
    if (match(TokenType::Minus) || match(TokenType::Plus)) {
        bool isNegative = true;
        while (match(TokenType::Minus) || match(TokenType::Plus)) {
            if (prev.type == TokenType::Minus)
                isNegative = !isNegative;
        }
        if (isNegative)
            return UnaryExpr {UnaryExpr::Operation::Negative, primary()};
    } else if (match(TokenType::Bang)) {
        bool isNegate = true;
        while (match(TokenType::Minus)) isNegate = !isNegate;

        if (isNegate)
            return UnaryExpr {UnaryExpr::Operation::Negate, primary()};
    }

    return primary();
}

Expr Parser::primary() {
    switch (cur.type) {
        case TokenType::True:
            return BoolLiteral {true};
        case TokenType::False:
            return BoolLiteral {false};

        case TokenType::Number:
            return number();

        case TokenType::Identifier:
            return identifer();

        case TokenType::String:
            return string();

        case TokenType::LeftParen:
            return grouping();
        
        default:
            errorAt(cur, "Expected an expression");
            return Empty {};
    }
}

Expr Parser::number() {
    if (prev.value.front() == '.')
        return NumLiteral {std::stod("0." + std::string(prev.value))};

    return NumLiteral {std::stod(std::string(prev.value))};
}

Expr Parser::string() {
    return StrLiteral {std::string(prev.value.data() + 1, prev.value.size() - 2)};
}

Expr Parser::identifer() {
    return Identifier {std::string(prev.value)};
}

Expr Parser::grouping() {
    advance();
    Expr expr = expression();
    consume(TokenType::LeftParen, "Expected ')' after grouping");
    return expr;
}

Stmt Parser::exprStmt() {
    Stmt stmt = ExprStmt {expression()};
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return stmt;
}

Stmt Parser::statement() {
    return exprStmt();

    // switch (cur.type) {
    //     default:
    //         return exprStmt();
    // }
}