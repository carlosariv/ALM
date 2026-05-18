#pragma once

#include "Token.h"
#include "Ast.h"

Token ast_start_token(Ast *node);
Token ast_end_token(Ast *node);

template<typename... Args>
void report_parser_error(Parser *P, std::format_string<Args...> fmt, Args&&... args) {
    std::print("syntax error: {},{}: ", P->current_line, P->current_col);
    std::println(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void report_error(Ast *node, std::format_string<Args...> fmt, Args&&... args) {
    Token start = {};
    if (node) {
        start = ast_start_token(node);
        end   = ast_end_token(node);
    }

    std::print("error: {},{}: ", start.start.line, start.start.col);
    std::println(fmt, std::forward<Args>(args)...);
}
