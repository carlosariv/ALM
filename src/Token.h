#pragma once

#include <string>
#include <string_view>

#include "BaseTypes.h"
#include "String.h"

struct Atom;

enum TokenKind {
    Token_Unknown,
    Token_EndOfFile,

    Token_Name,
    Token_Integer,
    Token_Floating,
    Token_String,

    Token_Ampersand,
    Token_At,
    Token_Bang,
    Token_Bar,
    Token_Caret,
    Token_Dollar,
    Token_Hash,
    Token_Plus,
    Token_Minus,
    Token_Star,
    Token_Percent,
    Token_Slash,

    Token_Tilde,
    Token_Squiggle,
    Token_Quote,

    Token_OpenParen,
    Token_CloseParen,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_Colon,
    Token_Semicolon,

    Token_Equal,
    Token_NotEqual,
    Token_Less,
    Token_Greater,
    Token_LessEqual,
    Token_GreaterEqual,

    Token_LeftShift,
    Token_RightShift,
    Token_And,
    Token_Or,

    Token_Comma,
    Token_Dot,
    Token_Ellipsis,
    Token_DotStar,
    Token_Arrow,
    Token_UnInit,

    Token_Assign_Begin,
    Token_Assign,
    Token_PlusAssign,
    Token_MinusAssign,
    Token_MulAssign,
    Token_DivAssign,
    Token_AndAssign,
    Token_OrAssign,
    Token_XorAssign,
    Token_ModAssign,
    Token_Assign_End,

    Token_KeywordBegin,
    Token_Struct,
    Token_Union,
    Token_Enum,
    Token_Using,
    Token_While,
    Token_For,
    Token_Do,
    Token_Break,
    Token_Continue,
    Token_Fallthrough,
    Token_If,
    Token_Else,
    Token_Then,
    Token_Ifcase,
    Token_Return,
    Token_KeywordEnd,

    Token_COUNT
};

struct SourcePos {
    i64 line = 0;
    i64 col = 0;
    usize index = 0;
    SourcePos() = default;
    SourcePos(i64 line, i64 col, usize index) : line(line), col(col), index(index) {}
};

struct Token {
    TokenKind kind = Token_Unknown;
    String string = {};
    SourcePos start;
    SourcePos end;
    union {
        u64 integer_value;
        f64 float_value;
        String string_value;
        Atom *name;
    };

    Token() {
        integer_value = 0;
    }

    // Token& operator=(const Token& other) {
    //     kind = other.kind;
    //     string = other.string;
    //     start = other.start;
    //     end = other.end;
    //     switch (kind) {
    //         case TOKEN_INTEGER:
    //             integer_value = other.integer_value;
    //             break;
    //         case TOKEN_FLOAT:
    //             float_value = other.float_value;
    //             break;
    //         case TOKEN_STRING:
    //             string_value = other.string_value;
    //             break;
    //         case TOKEN_NAME:
    //             name = other.name;
    //             break;
    //     }
    //     return (Token&)other;
    // }
    // Token(const Token &other) {
    //     *this = other;
    // }
};
