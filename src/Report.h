#pragma once

#include <print>
#include <format>

#include "Token.h"
#include "Ast.h"
#include "Parser.h"

Token ast_start_token(Ast *node);
Token ast_end_token(Ast *node);

template<typename... Args>
void report_parser_error(Parser *P, std::format_string<Args...> fmt, Args&&... args) {
    std::print("syntax error: {}: {},{}: ", P->current_file->absolute_path, P->current_line, P->current_col);
    std::println(fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void report_error(Ast *node, std::format_string<Args...> fmt, Args&&... args) {
    SourceFile *file = nullptr;
    SourcePos start = {};
    SourcePos end = {};
    if (node) {
        Token start_token = ast_start_token(node);
        Token end_token   = ast_end_token(node);

        file = start_token.file;
        start = start_token.start;
        end = end_token.end;
    }

    std::print("{}:{}:{}: error: ", file->absolute_path, start.line, start.col);
    std::println(fmt, std::forward<Args>(args)...);

    if (start.line == end.line) {
        u64 line_pos = start.index - start.col + 1;
        u8 *s = file->content.text + line_pos;
        u64 line_end = line_pos;
        while (*s && *s != '\n' && *s != '\r') {
            s++;
            line_end++;
        }

        String before = make_string(file->content.text + line_pos, start.col-1);
        String msg = make_string(file->content.text + start.index, end.index - start.index);
        String after = make_string(file->content.text + end.index, line_end - end.index);

        std::println("\t{}\x1b[31m{}\x1b[0m{}", before, msg, after);

        std::print("\t\x1b[2m");
        for (int i = 0; i < start.col - 1; i++) {
            std::print(" ");
        }

        for (int i = start.col; i < end.col; i++) {
            std::print("^");
        }
        std::println("\x1b[0m");
    }
}
