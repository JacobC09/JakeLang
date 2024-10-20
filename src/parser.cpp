#include "parser.h"

#include "debug.h"
#include "print.h"
#include "util.h"

Parser::Parser(std::string& source) : scanner(source) {
    hadError = false;
}

Ast Parser::parse() {
    Ast ast;

    advance();
    while (!isFinished()) {
        ast.body.push_back(statement());
    }

    return ast;
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
    if (!check(type)) return false;

    advance();
    return true;
}

template <typename... Args>
bool Parser::match(TokenType type, Args... args) {
    return match(type) || match(args...);
}

void Parser::errorAt(Token& token, std::string msg) {
    if (hadError) return;

    hadError = true;
    printf("Error at line %d\n", token.line);
    printf("    %s\n", msg.c_str());
}

bool Parser::isFinished() {
    return check(TokenType::EndOfFile) || hadError;
}

bool Parser::failed() {
    return hadError;
}

Expr Parser::expression() {
    return assignment();
}

Expr Parser::assignment() {
    Expr target = equality();

    while (match(TokenType::Equal, TokenType::PlusEqual, TokenType::MinusEqual, TokenType::SlashEqual, TokenType::AsteriskEqual, TokenType::CarretEqual)) {
        if (target.which() != Expr::which<Identifier>()) {
            errorAt(prev, "Assignment target must be an identifier");
            break;
        }

        TokenType opToken = prev.type;
        Expr right = equality();

        if (opToken != TokenType::Equal) {
            BinaryExpr::Operation op;

            switch (opToken) {
                case TokenType::PlusEqual:
                    op = BinaryExpr::Operation::Add;
                    break;
                case TokenType::MinusEqual:
                    op = BinaryExpr::Operation::Subtract;
                    break;
                case TokenType::AsteriskEqual:
                    op = BinaryExpr::Operation::Multiply;
                    break;
                case TokenType::SlashEqual:
                    op = BinaryExpr::Operation::Divide;
                    break;
                case TokenType::CarretEqual:
                    op = BinaryExpr::Operation::Exponent;
                    break;

                default:
                    break;
            }

            right = BinaryExpr{op, target, right};
        }

        target = AssignmentExpr{target.get<Identifier>(), right};
    }

    return target;
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
            default:
                break;
        }

        expr = BinaryExpr{op, expr, comparison()};
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
            default:
                break;
        }

        expr = BinaryExpr{op, expr, term()};
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
            default:
                break;
        }

        expr = BinaryExpr{op, expr, factor()};
    }

    return expr;
}

Expr Parser::factor() {
    Expr expr = exponent();

    while (match(TokenType::Asterisk, TokenType::Slash)) {
        BinaryExpr::Operation op;
        switch (prev.type) {
            case TokenType::Asterisk:
                op = BinaryExpr::Operation::Multiply;
                break;
            case TokenType::Slash:
                op = BinaryExpr::Operation::Divide;
                break;
            default:
                break;
        }

        expr = BinaryExpr{op, expr, exponent()};
    }

    return expr;
}

Expr Parser::exponent() {
    Expr expr = unary();

    while (match(TokenType::Carret)) {
        expr = BinaryExpr{BinaryExpr::Operation::Exponent, expr, unary()};
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
            return UnaryExpr{UnaryExpr::Operation::Negative, primary()};
    } else if (match(TokenType::Bang)) {
        bool isNegate = true;
        while (match(TokenType::Minus)) isNegate = !isNegate;

        if (isNegate)
            return UnaryExpr{UnaryExpr::Operation::Negate, primary()};
    }

    return primary();
}

Expr Parser::primary() {
    advance();

    switch (prev.type) {
        case TokenType::True:
            return BoolLiteral{true};
        case TokenType::False:
            return BoolLiteral{false};
        case TokenType::None:
            return NoneLiteral{};

        case TokenType::Number:
            return number();

        case TokenType::Identifier:
            return identifer();

        case TokenType::String:
            return string();

        case TokenType::LeftParen:
            return grouping();

        case TokenType::LeftBrace:
            return blockExpr();

        default:
            errorAt(prev, "Expected an expression");
            return Empty{};
    }
}

Expr Parser::number() {
    if (prev.value.front() == '.')
        return NumLiteral{std::stod("0." + std::string(prev.value))};

    return NumLiteral{std::stod(std::string(prev.value))};
}

Expr Parser::string() {
    return StrLiteral{std::string(prev.value.data() + 1, prev.value.size() - 2)};
}

Expr Parser::identifer() {
    return Identifier{std::string(prev.value)};
}

Expr Parser::grouping() {
    Expr expr = expression();
    consume(TokenType::RightParen, "Expected ')' after grouping");
    return expr;
}

Expr Parser::blockExpr() {
    Expr expr = BlockExpr{block()};
    return expr;
}

std::vector<Expr> Parser::exprList() {
    std::vector<Expr> values;

    while (!isFinished()) {
        values.push_back(expression());

        if (!match(TokenType::Comma))
            break;
    }

    return values;
}

std::vector<Stmt> Parser::block() {
    std::vector<Stmt> body;

    while (!check(TokenType::RightBrace) && !isFinished()) {
        body.push_back(statement());
    }

    consume(TokenType::RightBrace, "Expected '}' after block");
    return body;
}

Stmt Parser::statement() {
    switch (cur.type) {
        case TokenType::Print:
            return printStmt();

        case TokenType::If:
            return ifStmt();

        case TokenType::Loop:
            return loopBlock();

        case TokenType::While:
            return whileLoop();

        case TokenType::For:
            return forLoop();

        case TokenType::Return:
            return returnStmt();

        case TokenType::Func:
            return funcDeclaration();

        case TokenType::Var:
            return varDeclaration();

        case TokenType::Break:
            advance();
            consume(TokenType::Semicolon, "Expected ';' after break");
            return BreakStmt{};

        case TokenType::Continue:
            advance();
            consume(TokenType::Semicolon, "Expected ';' after continue");
            return ContinueStmt{};

        case TokenType::EndOfFile:
        case TokenType::Error:
            return Empty{};

        default:
            return exprStmt();
    }
}

Stmt Parser::exprStmt() {
    Stmt stmt = ExprStmt{expression()};
    if (prev.type != TokenType::RightBrace) {
        consume(TokenType::Semicolon, "Expected ';' after expression");
    }
    return stmt;
}

Stmt Parser::printStmt() {
    advance();
    std::vector<Expr> exprs = exprList();
    if (prev.type != TokenType::RightBrace) {
        consume(TokenType::Semicolon, "Expected ';' after print statement");
    }
    return PrintStmt{exprs};
}

Stmt Parser::ifStmt() {
    advance();
    Expr condition = expression();
    consume(TokenType::LeftBrace, "Expected '{' after if condition");
    std::vector<Stmt> body = block();
    std::vector<Stmt> orelse;

    if (match(TokenType::Else)) {
        if (check(TokenType::If)) {
            orelse.push_back(ifStmt());
        } else {
            consume(TokenType::LeftBrace, "Expected '{' after else clause");
            orelse = block();
        }
    }

    return IfStmt{condition, body, orelse};
}

Stmt Parser::loopBlock() {
    advance();
    consume(TokenType::LeftBrace, "Expected '{' after loop");
    return LoopBlock{block()};
}

Stmt Parser::whileLoop() {
    advance();
    Expr condition = expression();
    consume(TokenType::LeftBrace, "Expected '{' after while condition");
    return WhileLoop{condition, block()};
}

Stmt Parser::forLoop() {
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "For loop target must be an identifier");
        return Empty{};
    }
    Identifier target = identifer().get<Identifier>();
    consume(TokenType::In, "Expected 'in' after for loop target");
    Expr iterator = expression();
    consume(TokenType::LeftBrace, "Expected '{' after for iterator");
    return ForLoop{target, iterator, block()};
}

Stmt Parser::returnStmt() {
    advance();
    Expr value = expression();
    consume(TokenType::Semicolon, "Expected ';' after return statement");
    return ReturnStmt{value};
}

Stmt Parser::funcDeclaration() {
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "Function name must be an identifier");
        return Empty{};
    }
    Identifier name = identifer().get<Identifier>();
    consume(TokenType::LeftParen, "Expected '(' after function name");

    std::vector<Identifier> args;
    while (!isFinished() && !check(TokenType::RightParen)) {
        Expr expr = expression();
        if (expr.which() != Expr::which<Identifier>()) {
            errorAt(prev, "Expected argument identifiers");
            break;
        }
        args.push_back(expr.get<Identifier>());

        if (!match(TokenType::Comma))
            break;
    }

    consume(TokenType::RightParen, "Expected ')' after function arguments");
    consume(TokenType::LeftBrace, "Expected '{' before function body");
    return FuncDeclaration{name, args, block()};
}

Stmt Parser::varDeclaration() {
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "Function name must be an identifier");
        return Empty{};
    }
    Identifier name = identifer().get<Identifier>();
    Expr expr;
    if (match(TokenType::Equal)) {
        expr = expression();
    } else {
        expr = NoneLiteral{};
    }
    if (prev.type != TokenType::RightBrace) {
        consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    }
    return VarDeclaration{name, expr};
}
