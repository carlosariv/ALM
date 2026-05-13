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

struct Ast;

struct SourceFile {
    String path;
    String filename;
    String content;
};

struct Parser {
    Array<SourceFile*> files;

    SourceFile* current_file;
    Token current_token;
    usize current_line;
    usize current_col;
    isize stream_index;
    u8 *stream;

    int expr_level = 0;
    bool allow_type = false;
};


Token get_token(Parser *P);
Token next_token(Parser *P);


template<typename... Args>
void report_parser_error(Parser *P, std::format_string<Args...> fmt, Args&&... args) {
    // Optional: Add a prefix
    std::print(std::cerr, "LOG: "); 

    // Forward the original format string and arguments to std::println
    // using perfect forwarding (std::forward) to preserve argument types.
    std::println(std::cerr, fmt.get(), std::forward<Args>(args)...); 
}

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

inline void advance_char(Parser *P) {
    P->stream_index++;
    if (P->stream_index >= P->current_file->content.len) {
        P->stream_index = P->current_file->content.len - 1;
    }
    P->stream = P->current_file->content.text + P->stream_index;
}

inline void add_source(Parser *P, SourceFile* source) {
    P->files.add(source);
}


inline char peek_next_char(Parser *P) {
    if (end_of_file(P)) {
        return 0;
    }
    return P->current_file->content[P->stream_index + 1];
}

inline bool match_char(Parser *P, char ch) {
    return peek_char(P) == ch;
}

inline void advance_line(Parser *P) {
    while (!end_of_file(P)) {
        if (peek_char(P) == '\n') {
            advance_char(P);
            break;
        }
        advance_char(P);
    }
}


template<typename... Args>
void syntax_error(Parser *P, std::format_string<Args...> fmt, Args&&... args);

void init_parse_context(Parser *P, SourceFile *file);

Token expect_token(Parser *P, TokenKind token);

void parse(Parser *P);
AstFile *parse_file(Parser *P, SourceFile *file);
AstValueDecl *parse_decl(Parser *P);

AstBlockExpr *parse_block_expr(Parser *P);
Ast *parse_stmt(Parser *P);
Ast *parse_simple_stmt(Parser *P);
Ast *parse_expr_stmt(Parser *P);

AstName *parse_name(Parser *P);
Ast *parse_base_expr(Parser *P);
Ast *parse_primary_expr(Parser *P);
Ast *parse_unary_expr(Parser *P);
Ast *parse_binary_expr(Parser *P, Ast *lhs, int precedence);
Ast *parse_expr(Parser *P);


AstIfExpr *parse_if_expr(Parser *P);

Array<Ast*> parse_expr_list(Parser *P);
Array<Ast*> parse_name_list(Parser *P);

Ast *parse_type_defn(Parser *P);

AstProcLit *parse_proc_lit(Parser *P);
AstProcType *parse_proc_type(Parser *P);
AstStructType *parse_struct_type(Parser *P);

Ast *parse_type(Parser *P);

void init_global_parser();

