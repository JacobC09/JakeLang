#include "syntax/parser.h"
#include "debug.h"
#include "print.h"
#include "util.h"

Parser::Parser(std::string& src, std::string& path) : source(src), scanner(src), path(path) {
    hadError = false;
}

Ast Parser::parse() {
    Ast ast;
    ast.source = source;

    advance();
    while (!isFinished()) {
        ast.body.push_back(statement());
    }

    return ast;
}

bool Parser::failed() {
    return hadError;
}

Error Parser::getError() {
    return error;
}

void Parser::advance() {
    if (hadError) return;

    prev = cur;
    cur = scanner.nextToken();

    if (cur.type == TokenType::Error) {
        errorAt(cur, formatStr("Invalid Token: %s", cur.value));
    }
}

void Parser::errorAt(Token& token, std::string msg, std::string note) {
    if (hadError) return;
    errorAtView(token.view, msg, note);
}

void Parser::errorAtView(SourceView view, std::string msg, std::string note) {
    if (hadError) return;
    hadError = true;
    error = Error {view, "SyntaxError", msg, note, path};
}

void Parser::consume(TokenType type, std::string msg) {
    if (cur.type == type) {
        advance();
        return;
    }

    if (hadError) return;

    hadError = true;
    error = Error {SourceView {prev.view.index + prev.view.length, 1, prev.view.line, prev.view.column + prev.view.length}, "SyntaxError", msg, "here", path};
};

bool Parser::isFinished() {
    return check(TokenType::EndOfFile) || hadError;
}

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

Expr Parser::expression() {
    return assignment();
}

Expr Parser::assignment() {
    SourceView view = cur.view;
    Expr target = _or();

    while (match(TokenType::Equal, TokenType::PlusEqual, TokenType::MinusEqual, TokenType::SlashEqual, TokenType::AsteriskEqual, TokenType::CarretEqual)) {
        Token opToken = prev;
        Expr right = _or();

        if (opToken.type != TokenType::Equal) {
            BinaryExpr::Operation op;

            switch (opToken.type) {
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
            
            right = BinaryExpr{view | prev.view, opToken, op, target, right};
        }

        target = AssignmentExpr{view | prev.view, target, right};
    }

    return target;
}

Expr Parser::_or() {
    SourceView view = cur.view;
    Expr expr = _and();

    while (match(TokenType::Or)) {
        Token opToken = prev;
        Expr right = _and();
        expr = BinaryExpr{view | prev.view, opToken, BinaryExpr::Operation::Or, expr, right};
    }

    return expr;
}

Expr Parser::_and() {
    SourceView view = cur.view;
    Expr expr = equality();

    while (match(TokenType::And)) {
        Token opToken = prev;
        Expr right = equality();
        expr = BinaryExpr{view | prev.view, opToken, BinaryExpr::Operation::And, expr, right};
    }

    return expr;
}

Expr Parser::equality() {
    SourceView view = cur.view;
    Expr expr = comparison();

    while (match(TokenType::EqualEqual, TokenType::BangEqual)) {
        Token opToken = prev;
        Expr right = comparison();
        expr = BinaryExpr{view | prev.view, opToken, opToken.type == TokenType::EqualEqual ? BinaryExpr::Operation::Equal : BinaryExpr::Operation::NotEqual, expr, right};
    }

    return expr;
}

Expr Parser::comparison() {
    SourceView view = cur.view;
    Expr expr = term();

    while (match(TokenType::Greater, TokenType::Less, TokenType::LessEqual, TokenType::GreaterEqual)) {
        Token opToken = prev;
        BinaryExpr::Operation op;
        switch (opToken.type) {
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

        Expr right = term();
        expr = BinaryExpr{view | prev.view, opToken, op, expr, right};
    }

    return expr;
}

Expr Parser::term() {
    SourceView view = cur.view;
    Expr expr = factor();

    while (match(TokenType::Plus, TokenType::Minus, TokenType::Percent)) {
        Token opToken = prev;
        BinaryExpr::Operation op;
        switch (opToken.type) {
            case TokenType::Plus:
                op = BinaryExpr::Operation::Add;
                break;
            case TokenType::Minus:
                op = BinaryExpr::Operation::Subtract;
                break;
            case TokenType::Percent:
                op = BinaryExpr::Operation::Modulous;
                break;
            default:
                break;
        }

        Expr right = factor();
        expr = BinaryExpr{view | prev.view, opToken, op, expr, right};
    }

    return expr;
}

Expr Parser::factor() {
    SourceView view = cur.view;
    Expr expr = exponent();

    while (match(TokenType::Asterisk, TokenType::Slash)) {
        Token opToken = prev;
        Expr right = exponent();
        expr = BinaryExpr{view | prev.view, opToken, opToken.type == TokenType::Asterisk ? BinaryExpr::Operation::Multiply : BinaryExpr::Operation::Divide, expr, right};
    }

    return expr;
}

Expr Parser::exponent() {
    SourceView view = cur.view;
    Expr expr = unary();

    while (match(TokenType::Carret)) {
        Token opToken = prev;
        Expr right = unary();
        expr = BinaryExpr{view | prev.view, opToken, BinaryExpr::Operation::Exponent, expr, right};
    }

    return expr;
}

Expr Parser::unary() {
    SourceView view = cur.view;

    if (match(TokenType::Minus, TokenType::Plus)) {
        bool isNegative = true;
        while (match(TokenType::Minus, TokenType::Plus)) {
            if (prev.type == TokenType::Minus)
                isNegative = !isNegative;
        }
        if (isNegative) {
            Token opToken = prev;
            Expr expr = post();
            return UnaryExpr{view | prev.view, opToken, UnaryExpr::Operation::Negative, expr};
        }
    } else if (match(TokenType::Bang)) {
        bool isNegate = true;
        while (match(TokenType::Minus)) isNegate = !isNegate;

        if (isNegate) {
            Token opToken = prev;
            Expr expr = post();
            return UnaryExpr{view | prev.view, opToken, UnaryExpr::Operation::Negate, expr};
        }
    }

    return post();
}

Expr Parser::post() {
    SourceView view = cur.view;
    Expr expr = primary();

    while (match(TokenType::Dot, TokenType::LeftParen)) {
        if (prev.type == TokenType::Dot) {
            consume(TokenType::Identifier, "Expected identifier name after '.'");
            Identifier prop = identifer();
            expr = PropertyExpr{view | prev.view, expr, prop};
        } else {
            std::vector<Expr> args;
            if (!check(TokenType::RightParen)) {
                args = exprList();
            }

            consume(TokenType::RightParen, "Expected ')' after argument list");
            expr = CallExpr{view | prev.view, expr, args};
        }
    }

    return expr;
}

Expr Parser::primary() {
    advance();

    switch (prev.type) {
        case TokenType::True:
            return BoolLiteral{prev.view, true};
        case TokenType::False:
            return BoolLiteral{prev.view, false};
        case TokenType::None:
            return NoneLiteral{prev.view};

        case TokenType::Number:
            return number();

        case TokenType::Identifier:
            return identifer();

        case TokenType::String:
            return string();

        case TokenType::LeftParen:
            return grouping();

        default:
            errorAt(prev, "Expected an expression");
            return Empty{};
    }
}

NumLiteral Parser::number() {
    if (prev.value.front() == '.')
        return NumLiteral{prev.view, std::stod("0." + prev.value)};

    return NumLiteral{prev.view, std::stod(prev.value)};
}

StrLiteral Parser::string() {
    return StrLiteral{prev.view, std::string(prev.value.data() + 1, prev.value.length() - 2)};
}

Identifier Parser::identifer() {
    return Identifier{prev.view, prev.value};
}

Expr Parser::grouping() {
    Expr expr = expression();
    consume(TokenType::RightParen, "Expected ')' after grouping");
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
    SourceView view = cur.view;

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

        case TokenType::Type:
            return typeDeclaration();

        case TokenType::Func:
            return funcDeclaration();

        case TokenType::Var:
            return varDeclaration();

        case TokenType::LeftBrace: {
            advance();
            std::vector<Stmt> body = block();
            return BlockStmt{view | prev.view, body};
        }

        case TokenType::Break: {
            advance();
            consume(TokenType::Semicolon, "Expected ';' after break");
            return BreakStmt{view};
        }

        case TokenType::Continue: {
            advance();
            consume(TokenType::Semicolon, "Expected ';' after continue");
            return ContinueStmt{view};
        }

        case TokenType::Exit: {
            advance();
            consume(TokenType::Number, "Expected number after exit");
            NumLiteral code = number();
            Stmt stmt = ExitStmt{view | prev.view, code};
            consume(TokenType::Semicolon, "Expected ';' exit code");
            return stmt;
        }

        case TokenType::EndOfFile:
        case TokenType::Error:
            return Empty{};

        default:
            return exprStmt();
    }
}

Stmt Parser::exprStmt() {
    SourceView view = cur.view;
    Expr expr = expression();
    Stmt stmt = ExprStmt{view | prev.view, expr};
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return stmt;
}

Stmt Parser::printStmt() {
    SourceView view = cur.view;
    advance();
    std::vector<Expr> exprs = exprList();
    Stmt stmt = PrintStmt{view | prev.view, exprs};
    consume(TokenType::Semicolon, "Expected ';' after print statement");
    return stmt;
}

Stmt Parser::ifStmt() {
    SourceView view = cur.view;
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

    return IfStmt{view | prev.view, condition, body, orelse};
}

Stmt Parser::loopBlock() {
    SourceView view = cur.view;
    advance();
    consume(TokenType::LeftBrace, "Expected '{' after loop");
    std::vector<Stmt> body = block();
    return LoopBlock{view | prev.view, body};
}

Stmt Parser::whileLoop() {
    SourceView view = cur.view;
    advance();
    Expr condition = expression();
    consume(TokenType::LeftBrace, "Expected '{' after while condition");
    std::vector<Stmt> body = block();
    return WhileLoop{view | prev.view, condition, body};
}

Stmt Parser::forLoop() {
    SourceView view = cur.view;
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "For loop target must be an identifier");
        return Empty{};
    }
    Identifier target = identifer();
    consume(TokenType::In, "Expected 'in' after for loop target");
    Expr iterator = expression();
    consume(TokenType::LeftBrace, "Expected '{' after for iterator");
    std::vector<Stmt> body = block();
    return ForLoop{view | prev.view, target, iterator, body};
}

Stmt Parser::returnStmt() {
    SourceView view = cur.view;
    advance();
    Expr value = NoneLiteral{};
    if (!match(TokenType::Semicolon)) {
        value = expression();
        view = view | prev.view;
        consume(TokenType::Semicolon, "Expected ';' after return statement");
    }
    return ReturnStmt{view, value};
}

Stmt Parser::typeDeclaration() {
    SourceView view = cur.view;
    advance();
    consume(TokenType::Identifier, "Type name must be an idenfitier");
    Identifier name = identifer();
    
    std::vector<Identifier> parents;
    if (match(TokenType::Semicolon)) {
        do {
            consume(TokenType::Identifier, "Parent must by an identifier");
            parents.push_back(identifer());
        } while (match(TokenType::Comma));
    }

    consume(TokenType::LeftBrace, "Expected '{' before type body");

    std::vector<Stmt> methods;
    while (!check(TokenType::RightBrace) && !isFinished()) {
        methods.push_back(methodDeclaration());
    }

    consume(TokenType::RightBrace, "Expected '}' after block");
    return TypeDeclaration{view | prev.view, name, parents, methods};
}

Stmt Parser::funcDeclaration() {
    SourceView view = cur.view;
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "Function name must be an identifier");
        return Empty{};
    }
    Identifier name = identifer();
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
    std::vector<Stmt> body = block();
    return FuncDeclaration{view | prev.view, name, args, body};
}

Stmt Parser::methodDeclaration() {
    SourceView view = cur.view;
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "Method name must be an identifier");
        return Empty{};
    }
    Identifier name = identifer();
    consume(TokenType::LeftParen, "Expected '(' after method name");

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

    consume(TokenType::RightParen, "Expected ')' after method arguments");
    consume(TokenType::LeftBrace, "Expected '{' before method body");
    std::vector<Stmt> body = block();
    return FuncDeclaration{view | prev.view, name, args, body};
}

Stmt Parser::varDeclaration() {
    SourceView view = cur.view;
    advance();
    if (!match(TokenType::Identifier)) {
        errorAt(cur, "Function name must be an identifier");
        return Empty{};
    }
    Identifier name = identifer();
    Expr expr;
    if (match(TokenType::Equal)) {
        expr = expression();
    } else {
        expr = Empty{};
    }
    Stmt stmt = VarDeclaration{view | prev.view, name, expr};
    consume(TokenType::Semicolon, "Expected ';' after variable declaration");
    return stmt;
}
