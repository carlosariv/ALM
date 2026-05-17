#pragma once

#include <string>
#include <span>

#include <iostream>
#include <utility>
#include <print>

#include "BaseTypes.h"
#include "Array.h"
#include "Token.h"
#include "AST.h"

struct SourceFile {
    String path;
    String filename;
    String content;
};

struct Parser {
    Array<SourceFile*> files;

    SourceFile* current_file;
    Token current_token;
    usize current_line = 1;
    usize current_col = 1;
    isize stream_index = 0;
    u8 *stream;

    int expr_level = 0;
    bool allow_type = false;

    Ast *control_target = nullptr;
    BlockExpr *block = nullptr;
};


Token get_token(Parser *P);
Token next_token(Parser *P);
void advance_char(Parser *P);


inline TokenKind peek_token(Parser *P) {
    return P->current_token.kind;
}

inline bool is_token(Parser *P, TokenKind kind) {
    return P->current_token.kind == kind;
}

inline bool match_token(Parser *P, TokenKind kind) {
    if (is_token(P, kind)) {
        next_token(P);
        return true;
    }
    return false;
}

inline char peek_char(Parser *P) {
    return P->current_file->content[P->stream_index];
}

inline bool end_of_file(Parser *P) {
    return peek_char(P) == '\0';
}

inline char peek_next_char(Parser *P) {
    if (end_of_file(P)) {
        return 0;
    }
    return P->current_file->content[P->stream_index + 1];
}

inline void add_source(Parser *P, SourceFile* source) {
    P->files.add(source);
}

inline bool match_char(Parser *P, char ch) {
    return peek_char(P) == ch;
}

inline void advance_line(Parser *P) {
    isize line = P->current_line;
    while (!end_of_file(P)) {
        if (P->current_line > line) {
            break;
        }
        advance_char(P);
    }
}

Token expect_token(Parser *P, TokenKind token);

AstFile *parse_file(Parser *P, SourceFile *file);

AstName *parse_name(Parser *P);
Array<Ast*> parse_expr_list(Parser *P);
BlockExpr *parse_block_expr(Parser *P);
Ast *parse_expr(Parser *P);

Ast *parse_stmt(Parser *P);
Ast *parse_simple_stmt(Parser *P);

ProcTypeDefn *parse_proc_type(Parser *P);
StructTypeDefn *parse_struct_type(Parser *P);

Ast *parse_type(Parser *P);
Ast *parse_operand(Parser *P);


