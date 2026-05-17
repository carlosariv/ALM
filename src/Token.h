#pragma once

#include "BaseTypes.h"
#include "String.h"

struct Atom;

#define TOKEN_LIST() \
X(Token_Unknown, "Unknown") \
X(Token_EndOfFile, "EndOfFile") \
X(Token_Name, "Name") \
X(Token_Integer, "Integer") \
X(Token_Floating, "Floating") \
X(Token_String, "String") \
X(Token_Ampersand, "&") \
X(Token_At, "@") \
X(Token_Bang, "!") \
X(Token_Bar, "|") \
X(Token_Caret, "^") \
X(Token_Dollar, "$") \
X(Token_Hash, "#") \
X(Token_Plus, "_") \
X(Token_Minus, "-") \
X(Token_Star, "*") \
X(Token_Percent, "%") \
X(Token_Slash, "/") \
X(Token_Tilde, "`") \
X(Token_Squiggle, "~") \
X(Token_Quote, "\"") \
X(Token_OpenParen, "(") \
X(Token_CloseParen, ")") \
X(Token_OpenBrace, "{") \
X(Token_CloseBrace, "}") \
X(Token_OpenBracket, "[") \
X(Token_CloseBracket, "]") \
X(Token_Colon, ":") \
X(Token_Semicolon, ";") \
X(Token_Equal, "==") \
X(Token_NotEqual, "!=") \
X(Token_Less, "<") \
X(Token_Greater, ">") \
X(Token_LessEqual, "<=") \
X(Token_GreaterEqual, ">=") \
X(Token_LeftShift, "<<") \
X(Token_RightShift, ">>") \
X(Token_And, "&&") \
X(Token_Or, "||") \
X(Token_Comma, ",") \
X(Token_Dot, ".") \
X(Token_Ellipsis, "..") \
X(Token_DotStar, ".*") \
X(Token_Arrow, "->") \
X(Token_UnInit, "---") \
X(Token_Assign_Begin, "Assign_Begin") \
X(Token_Assign, "=") \
X(Token_PlusAssign, "+=") \
X(Token_MinusAssign, "-=") \
X(Token_MulAssign, "*=") \
X(Token_DivAssign, "/=") \
X(Token_AndAssign, "&=") \
X(Token_OrAssign, "|=") \
X(Token_XorAssign, "^=") \
X(Token_ModAssign, "%=") \
X(Token_Assign_End, "Assign_End") \
X(Token_KeywordBegin, "KeywordBegin") \
X(Token_Struct, "struct") \
X(Token_Union, "union") \
X(Token_Enum, "enum") \
X(Token_Using, "using") \
X(Token_While, "while") \
X(Token_For, "for") \
X(Token_Do, "do") \
X(Token_Break, "break") \
X(Token_Continue, "continue") \
X(Token_Fallthrough, "fallthrough") \
X(Token_If, "if") \
X(Token_Else, "else") \
X(Token_Then, "then") \
X(Token_Case, "case") \
X(Token_Of, "of") \
X(Token_Return, "return") \
X(Token_KeywordEnd, "KeywordEnd") \


#define OPERATOR_LIST() \
X(Operator_Nil, "Nil") \
X(Operator_UnaryPlus, "unary '+'") \
X(Operator_Not, "!") \
X(Operator_Negate, "unary '-'") \
X(Operator_AddressOf, "AddressOf") \
X(Operator_IndexOf, "[]") \
X(Operator_Deref, ".*") \
X(Operator_Add, "binary '+'") \
X(Operator_Sub, "binray '-'") \
X(Operator_Mult, "binary '*'") \
X(Operator_Div, "/") \
X(Operator_Mod, "%") \
X(Operator_Equal, "==") \
X(Operator_NotEqual, "!=") \
X(Operator_Less, "<") \
X(Operator_Greater, ">") \
X(Operator_LessEqual, "<=") \
X(Operator_GreaterEqual, ">=") \
X(Operator_LeftShift, "<<") \
X(Operator_RightShift, ">>") \
X(Operator_Xor, "^") \
X(Operator_BitwiseAnd, "&") \
X(Operator_BitwiseOr, "|") \
X(Operator_And, "&&") \
X(Operator_Or, "||") \
X(Operator_Cast, "cast") \


#define X(K,S) K,
enum TokenKind {
    TOKEN_LIST()
    Token_COUNT
};
#undef X

#define X(K, S) K,
enum Operator {
    OPERATOR_LIST()
    Operator_COUNT
};
#undef X

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
};
