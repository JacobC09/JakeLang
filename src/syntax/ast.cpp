#include "syntax/ast.h"

SourceView operator|(const SourceView& left, const SourceView& right) {
    int index = std::min(left.index, right.index);
    int length = std::max(left.index + left.length, right.index + right.length) - index;
    int line = std::min(left.line, right.line);
    int column = left.index < right.index ? left.column : right.column;

    return SourceView{index, length, line, column};
}

SourceView getSourceView(Expr& expr) {
    return expr.match([](const auto& node) -> SourceView {
        if constexpr (std::is_base_of_v<AstNode, std::decay_t<decltype(node)>>) {
            return node.view;
        }
        return SourceView{};
    });
}

SourceView getSourceView(Stmt& stmt) {
    return stmt.match([](const auto& node) -> SourceView {
        if constexpr (std::is_base_of_v<AstNode, std::decay_t<decltype(node)>>) {
            return node.view;
        }
        return SourceView{};
    });
}
