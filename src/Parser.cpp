#include <assert.h>
#include <unordered_map>

#include "Atom.h"
#include "AST.h"
#include "String.h"
#include "Token.h"
#include "Parser.h"

std::unordered_map<String, TokenKind, StringHasher> keyword_map;

String g_token_strings[Token_COUNT];
String g_operator_strings[Operator_COUNT];


template<typename... Args>
void syntax_error(Parser *P, std::format_string<Args...> fmt, Args&&... args) {
    std::print("syntax error: {},{}: ", P->current_line, P->current_col);
    std::println(fmt, std::forward<Args>(args)...);
}

String string_from_token(TokenKind token) {
    return g_token_strings[token];
}

String string_from_operator(Operator op) {
    return g_operator_strings[op];
}

AstValueDecl *parse_decl(Parser *P);

AstBlockExpr *parse_block_expr(Parser *P);
Ast *parse_stmt(Parser *P);
Ast *parse_simple_stmt(Parser *P);
Ast *parse_expr_stmt(Parser *P);


Array<Ast*> parse_expr_list(Parser *P);
Array<Ast*> parse_name_list(Parser *P);

Ast *parse_type_defn(Parser *P);

AstProcLit *parse_proc_lit(Parser *P);
AstProcType *parse_proc_type(Parser *P);
AstStructType *parse_struct_type(Parser *P);


Operator unary_operator_from_token(TokenKind token) {
    switch (token) {
        default:
            return Operator_Nil;
        case Token_Plus:
            return Operator_UnaryPlus;
        case Token_Minus:
            return Operator_Negate;
        case Token_Star:
            return Operator_AddressOf;
    }
}

int to_digit(char c) {
    switch (c) {
        default:
            return -1;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            return c - 'a' + 10;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            return c - 'A' + 10;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return c - '0';
    }
}

void set_keyword(String string, TokenKind token) {
    keyword_map.insert({string, token});
}

void init_global_parser() {
    set_keyword(STRZ("struct"), Token_Struct);
    set_keyword(STRZ("union"), Token_Union);
    set_keyword(STRZ("enum"), Token_Enum);
    set_keyword(STRZ("using"), Token_Using);
    set_keyword(STRZ("while"), Token_While);
    set_keyword(STRZ("for"), Token_For);
    set_keyword(STRZ("do"), Token_Do);
    set_keyword(STRZ("if"), Token_If);
    set_keyword(STRZ("else"), Token_Else);
    set_keyword(STRZ("then"), Token_Then);
    set_keyword(STRZ("ifcase"), Token_Ifcase);
    set_keyword(STRZ("return"), Token_Return);

    g_operator_strings[Operator_Nil] = STRZ("Nil");
    g_operator_strings[Operator_UnaryPlus] = STRZ("UnaryPlus");
    g_operator_strings[Operator_Not] = STRZ("Not");
    g_operator_strings[Operator_Negate] = STRZ("Negate");
    g_operator_strings[Operator_AddressOf] = STRZ("AddressOf");
    g_operator_strings[Operator_IndexOf] = STRZ("IndexOf");
    g_operator_strings[Operator_Deref] = STRZ("Deref");
    g_operator_strings[Operator_Add] = STRZ("Add");
    g_operator_strings[Operator_Sub] = STRZ("Sub");
    g_operator_strings[Operator_Mult] = STRZ("Mult");
    g_operator_strings[Operator_Div] = STRZ("Div");
    g_operator_strings[Operator_Mod] = STRZ("Mod");
    g_operator_strings[Operator_Equal] = STRZ("Equal");
    g_operator_strings[Operator_Less] = STRZ("Less");
    g_operator_strings[Operator_Greater] = STRZ("Greater");
    g_operator_strings[Operator_LessEqual] = STRZ("LessEqual");
    g_operator_strings[Operator_GreaterEqual] = STRZ("GreaterEqual");
    g_operator_strings[Operator_LeftShift] = STRZ("LeftShift");
    g_operator_strings[Operator_RightShift] = STRZ("RightShift");
    g_operator_strings[Operator_Xor] = STRZ("Xor");
    g_operator_strings[Operator_BitwiseAnd] = STRZ("BitwiseAnd");
    g_operator_strings[Operator_BitwiseOr] = STRZ("BitwiseOr");
    g_operator_strings[Operator_And] = STRZ("And");
    g_operator_strings[Operator_Or] = STRZ("Or");
    g_operator_strings[Operator_Cast] = STRZ("Cast");

    for (int i = Operator_Nil; i < Operator_COUNT; i++) {
        assert(g_operator_strings[i].len != 0);
    }

    g_token_strings[Token_Unknown] = STRZ("Unknown");
    g_token_strings[Token_EndOfFile] = STRZ("EndOfFile");
    g_token_strings[Token_Name] = STRZ("Name");
    g_token_strings[Token_Integer] = STRZ("Integer");
    g_token_strings[Token_Floating] = STRZ("Floating");
    g_token_strings[Token_String] = STRZ("String");
    g_token_strings[Token_Ampersand] = STRZ("&");
    g_token_strings[Token_At] = STRZ("@");
    g_token_strings[Token_Bang] = STRZ("!");
    g_token_strings[Token_Bar] = STRZ("|");
    g_token_strings[Token_Caret] = STRZ("^");
    g_token_strings[Token_Dollar] = STRZ("$");
    g_token_strings[Token_Hash] = STRZ("#");
    g_token_strings[Token_Plus] = STRZ("_");
    g_token_strings[Token_Minus] = STRZ("-");
    g_token_strings[Token_Star] = STRZ("*");
    g_token_strings[Token_Percent] = STRZ("%");
    g_token_strings[Token_Slash] = STRZ("/");
    g_token_strings[Token_Tilde] = STRZ("`");
    g_token_strings[Token_Squiggle] = STRZ("~");
    g_token_strings[Token_Quote] = STRZ("\"");
    g_token_strings[Token_OpenParen] = STRZ("(");
    g_token_strings[Token_CloseParen] = STRZ(")");
    g_token_strings[Token_OpenBrace] = STRZ("{");
    g_token_strings[Token_CloseBrace] = STRZ("}");
    g_token_strings[Token_OpenBracket] = STRZ("[");
    g_token_strings[Token_CloseBracket] = STRZ("]");
    g_token_strings[Token_Colon] = STRZ(":");
    g_token_strings[Token_Semicolon] = STRZ(";");
    g_token_strings[Token_Equal] = STRZ("==");
    g_token_strings[Token_Less] = STRZ("<");
    g_token_strings[Token_Greater] = STRZ(">");
    g_token_strings[Token_LessEqual] = STRZ("<=");
    g_token_strings[Token_GreaterEqual] = STRZ(">=");
    g_token_strings[Token_LeftShift] = STRZ("<<");
    g_token_strings[Token_RightShift] = STRZ(">>");
    g_token_strings[Token_And] = STRZ("&&");
    g_token_strings[Token_Or] = STRZ("||");
    g_token_strings[Token_Comma] = STRZ(",");
    g_token_strings[Token_Dot] = STRZ(".");
    g_token_strings[Token_Ellipsis] = STRZ("..");
    g_token_strings[Token_DotStar] = STRZ(".*");
    g_token_strings[Token_Arrow] = STRZ("->");
    g_token_strings[Token_Assign_Begin] = STRZ("Assign_Begin");
    g_token_strings[Token_Assign] = STRZ("=");
    g_token_strings[Token_PlusAssign] = STRZ("+=");
    g_token_strings[Token_MinusAssign] = STRZ("-=");
    g_token_strings[Token_MulAssign] = STRZ("*=");
    g_token_strings[Token_DivAssign] = STRZ("/=");
    g_token_strings[Token_AndAssign] = STRZ("&=");
    g_token_strings[Token_OrAssign] = STRZ("|=");
    g_token_strings[Token_XorAssign] = STRZ("^=");
    g_token_strings[Token_ModAssign] = STRZ("%=");
    g_token_strings[Token_Assign_End] = STRZ("Assign_End");
    g_token_strings[Token_KeywordBegin] = STRZ("KeywordBegin");
    g_token_strings[Token_Struct] = STRZ("struct");
    g_token_strings[Token_Union] = STRZ("union");
    g_token_strings[Token_Enum] = STRZ("enum");
    g_token_strings[Token_Using] = STRZ("using");
    g_token_strings[Token_While] = STRZ("while");
    g_token_strings[Token_For] = STRZ("for");
    g_token_strings[Token_Do] = STRZ("do");
    g_token_strings[Token_If] = STRZ("if");
    g_token_strings[Token_Else] = STRZ("else");
    g_token_strings[Token_Then] = STRZ("then");
    g_token_strings[Token_Ifcase] = STRZ("ifcase");
    g_token_strings[Token_Return] = STRZ("return");
    g_token_strings[Token_KeywordEnd] = STRZ("KeywordEnd");

    for (int i = Token_Unknown; i < Token_COUNT; i++) {
        assert(g_token_strings[i].len != 0);
    }
}

Token get_token(Parser *P) {
    return P->current_token;
}

Token expect_token(Parser *P, TokenKind token) {
    if (is_token(P, token)) {
        Token token = P->current_token;
        next_token(P);
        return token;
    } else {
        syntax_error(P, "expected {}, got {}", string_from_token(token), string_from_token(peek_token(P)));
        return {};
    }
}

void init_parser_context(Parser *P, SourceFile *file) {
    P->current_file = file;
    P->current_line = 0;
    P->current_col = 0;
    P->stream_index = 0;
    P->stream = file->content.text;
    P->current_token = {};
    next_token(P);
}

void parse(Parser *P) {
    for (SourceFile *file : P->files) {
        AstFile *ast_file = parse_file(P, file);
    }
}

AstName *parse_name(Parser *P) {
    if (is_token(P, Token_Name)) {
        AstName *name = ast_new<AstName>();
        name->name = P->current_token.name;
        name->token = P->current_token;
        next_token(P);
        return name;
    } else {
        return nullptr;
    }
}

AstFile *parse_file(Parser *P, SourceFile *file) {
    init_parser_context(P, file);

    AstFile *ast_file = ast_new<AstFile>();

    while (peek_token(P) != Token_EndOfFile) {
        Ast *decl = parse_stmt(P);
        if (!decl) break;

        ast_file->decls.add(decl);
    }

    return ast_file;
}

Enumerator *parse_enumerator(Parser *P) {
    AstName *name = parse_name(P);

    if (!name) {
        return nullptr;
    }

    Ast *value = nullptr;

    if (match_token(P, Token_Assign)) {
        value = parse_expr(P);
    }

    Enumerator *enumerator = ast_new<Enumerator>();
    enumerator->name = name;
    enumerator->value = value;

    return enumerator;
}

AstEnumType *parse_enum_type(Parser *P) {
    Token token = expect_token(P, Token_Enum);

    Token open = expect_token(P, Token_OpenBrace);

    AstEnumType *enum_type = ast_new<AstEnumType>();

    while (!is_token(P, Token_CloseBrace)) {
        Enumerator *enumerator = parse_enumerator(P);

        if (!enumerator) break;

        enum_type->members.add(enumerator);

        if (!match_token(P, Token_Comma)) {
            break;
        }
    }

    Token close = expect_token(P, Token_CloseBrace);

    enum_type->token = token;
    enum_type->open = open;
    enum_type->close = close;

    return enum_type;
}

AstStructType *parse_struct_type(Parser *P) {
    Token token = expect_token(P, Token_Struct);

    Token open = expect_token(P, Token_OpenBrace);

    AstStructType *struct_type = ast_new<AstStructType>();

    while (!is_token(P, Token_CloseBrace)) {
        Ast *stmt = parse_simple_stmt(P);
        if (!stmt) break;

        struct_type->members.add(stmt);
    }

    Token close = expect_token(P, Token_CloseBrace);

    struct_type->token = token;
    struct_type->open = open;
    struct_type->close = close;
    return struct_type;
}

AstIfExpr *parse_if_expr(Parser *P) {
    Token token = expect_token(P, Token_If);

    Ast *condition = parse_expr(P);

    Ast *then_expr = parse_expr(P);

    AstIfExpr *if_expr = ast_new<AstIfExpr>();
    if_expr->condition = condition;
    if_expr->then_expr = then_expr;
    if_expr->token = token;

    AstIfExpr *prev_if = if_expr;

    while (is_token(P, Token_Else)) {
        Token e = expect_token(P, Token_Else);

        condition = nullptr;

        if (is_token(P, Token_If)) {
            Token i = expect_token(P, Token_If);
            condition = parse_expr(P);
        }

        then_expr = parse_expr(P);

        AstIfExpr *elif = ast_new<AstIfExpr>();
        elif->condition = condition;
        elif->then_expr = then_expr;
        elif->else_if = nullptr;

        prev_if->else_if = elif;
        prev_if = elif;
    }

    return if_expr;
}

AstBlockExpr *parse_block_expr(Parser *P) {
    Token open = expect_token(P, Token_OpenBrace);

    AstBlockExpr *block = ast_new<AstBlockExpr>();

    while (!is_token(P, Token_CloseBrace)) {
        Ast *stmt = parse_stmt(P);
        if (!stmt) break;

        block->statements.add(stmt);
    }

    Token close = expect_token(P, Token_CloseBrace);

    return block;
}

Ast *parse_operand(Parser *P) {
    Token token = P->current_token;
    switch (token.kind) {
        case Token_Name: {
            next_token(P);
            AstName *name =  ast_new<AstName>();
            name->token = token;
            name->name =  token.name;
            return name;
        }

        case Token_Integer:
        case Token_Floating:
        case Token_String: {
            next_token(P);

            AstLiteralExpr *expr = ast_new<AstLiteralExpr>();
            if (token.kind == Token_Integer) {
                expr->literal_kind = Literal_Integer;
            } else if (token.kind == Token_Floating) {
                expr->literal_kind = Literal_Floating;
            } else if (token.kind == Token_String) {
                expr->literal_kind = Literal_String;
            }
            expr->token = token;
            return expr;
        }

        case Token_OpenParen: {
            Token open = next_token(P);
            Ast *elem = parse_expr(P);
            Token close = expect_token(P, Token_CloseParen);

            AstParenExpr *paren = ast_new<AstParenExpr>();
            paren->expr = elem;
            paren->open = open;
            paren->close = close;
            return paren;
        }

        case Token_OpenBrace:
            return parse_block_expr(P);

        case Token_If:
            return parse_if_expr(P);

        case Token_Struct:
            return parse_struct_type(P);

        case Token_Enum:
            return parse_enum_type(P);
    }
    return nullptr;
}

Array<Ast*> parse_expr_list(Parser *P) {
    Array<Ast*> list;

    for (;;) {
        Ast *expr = parse_expr(P);

        if (!expr) break;

        list.add(expr);

        if (match_token(P, Token_Comma)) {
            break;
        }
    }
    return list;
}

AstSubscriptExpr *parse_subscript_expr(Parser *P, Ast *operand) {
    Token open = expect_token(P, Token_OpenBracket);

    Ast *value = parse_expr(P);

    Token close = expect_token(P, Token_CloseBracket);

    AstSubscriptExpr *sub = ast_new<AstSubscriptExpr>();
    sub->operand = operand;
    sub->value = value;
    sub->open = open;
    sub->close = close;
    return sub;
}

AstCompoundLiteral *parse_compound_literal(Parser *P, Ast *operand) {
    Token open = expect_token(P, Token_OpenBrace);

    Array<Ast*> expr_list = parse_expr_list(P);

    Token close = expect_token(P, Token_CloseBrace);

    AstCompoundLiteral *compound = ast_new<AstCompoundLiteral>();
    compound->operand = operand;
    compound->open = open;
    compound->close = close;
    compound->initializer_list = expr_list;
    return compound;
}

Ast *parse_primary_expr(Parser *P, Ast *operand) {
    if (!operand) return nullptr;

    bool loop = true;
    while (loop) {
        switch (peek_token(P)) {
            default:
                loop = false;
                break;

            case Token_OpenParen: {
                Token open = expect_token(P, Token_OpenParen);

                Array<Ast*> arguments = parse_expr_list(P);

                Token close = expect_token(P, Token_CloseParen);
_
                AstCallExpr *call = ast_new<AstCallExpr>();
                call->arguments = arguments;
                call->operand = operand;
                operand = call;
                break;
            }

            case Token_OpenBracket: {
                operand = parse_subscript_expr(P, operand);
                break;
            }

            case Token_OpenBrace: {
                operand = parse_compound_literal(P, operand);
                break;
            }
        }
    }

    return operand;
}

Ast *parse_unary_expr(Parser *P) {
    switch (peek_token(P)) {
        default:
            return parse_primary_expr(P, parse_operand(P));

        case Token_Minus:
        case Token_Star:
        case Token_Plus: {
            Token token = P->current_token;
            Ast *operand = parse_unary_expr(P);
            AstUnaryExpr *unary = ast_new<AstUnaryExpr>();
            unary->op = unary_operator_from_token(token.kind);
            unary->operand = operand;
            unary->token = token;
            return unary;
        }
    }
}

Ast *parse_binary_expr(Parser *P, Ast *lhs, int current_prec) {
    return lhs;
    for (;;) {
        Token op_token = get_token(P);
    }
}

Ast *parse_expr(Parser *P) {
    Ast *lhs = parse_unary_expr(P);
    return parse_binary_expr(P, lhs, 0);
}

Ast *parse_simple_stmt(Parser *P) {
    Array<Ast*> lhs = parse_expr_list(P);

    if (lhs.count == 0) return nullptr;

    if (is_token(P, Token_Colon)) {
    }

    return lhs[0];
}

Ast *parse_stmt(Parser *P) {
    Ast *stmt = nullptr;
    switch (peek_token(P)) {
    default:
        stmt = parse_simple_stmt(P);
        break;

        case Token_Else:
            syntax_error(P, "illegal else without matching if");
            break;

        case Token_While:
        case Token_Do:
        case Token_For:
            break;
    }

    return stmt;
}

Token next_token(Parser *P) {
    Token tok = {};

scan_begin:
    tok.start.line = P->current_line;
    tok.start.col = P->current_col;
    tok.start.index = P->stream_index;

    #define TOKCASE(C,T) case C: advance_char(P); tok.kind = T; break

    switch (peek_token(P)) {
        default:
            tok.kind = Token_Unknown;
            break;

            TOKCASE(0, Token_EndOfFile);

            TOKCASE('@', Token_At);
            TOKCASE('#', Token_Hash);
            TOKCASE('$', Token_Dollar);
            TOKCASE('%', Token_Percent);
            TOKCASE('^', Token_Caret);
            TOKCASE('*', Token_Star);
            TOKCASE('(', Token_OpenParen);
            TOKCASE(')', Token_CloseParen);
            TOKCASE('[', Token_OpenBracket);
            TOKCASE(']', Token_CloseBracket);
            TOKCASE('{', Token_OpenBrace);
            TOKCASE('}', Token_CloseBrace);
            TOKCASE('~', Token_Squiggle);
            TOKCASE('`', Token_Tilde);
            TOKCASE('\'', Token_Quote);
            TOKCASE(':', Token_Colon);
            TOKCASE(';', Token_Semicolon);
            TOKCASE(',', Token_Comma);

        case '!':
            advance_char(P);
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_NotEqual;
            } else {
                tok.kind = Token_Bang;
            }
            break;

        case '.':
            advance_char(P);
            if (peek_char(P) == '.') {
                advance_char(P);
                tok.kind = Token_Ellipsis;
            } else if (peek_char(P) == '*') {
                advance_char(P);
                tok.kind = Token_DotStar;
            } else {
                tok.kind = Token_Dot;
            }
            break;

        case '-':
            advance_char(P);
            if (peek_char(P) == '>') {
                advance_char(P);
                tok.kind = Token_Arrow;
            } else if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_MinusAssign;
            }
            break;

        case '+':
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_PlusAssign;
            } else {
                tok.kind = Token_Plus;
            }
            break;

        case '/':
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_DivAssign;
            } else {
                tok.kind = Token_Slash;
            }
            break;

        case '=':
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_Equal;
            } else {
                tok.kind = Token_Assign;
            }
            break;

        case '|':
            if (peek_char(P) == '|') {
                advance_char(P);
                tok.kind = Token_Or;
            } else if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_OrAssign;
            } else {
                tok.kind = Token_Bar;
            }
            break;


        case '&':
            if (peek_char(P) == '&') {
                advance_char(P);
                tok.kind = Token_And;
            } else if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_AndAssign;
            } else {
                tok.kind = Token_Ampersand;
            }
            break;

        case '<':
            if (peek_char(P) == '<') {
                advance_char(P);
                tok.kind = Token_LeftShift;
            } else if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_LessEqual;
            } else {
                tok.kind = Token_Less;
            }
            break;

        case '>':
            if (peek_char(P) == '>') {
                advance_char(P);
                tok.kind = Token_RightShift;
            } else if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_GreaterEqual;
            } else {
                tok.kind = Token_Greater;
            }
            break;

        case ' ': case '\n': case '\r': case '\t': case '\f':
            while (isspace(peek_char(P))) {
                advance_char(P);
            }
            goto scan_begin;

        case '\"': {
            while (peek_char(P) != '"') {
                advance_char(P);
            }

            int len = P->stream_index - tok.start.index;
            String string = make_string(P->current_file->content.text + tok.start.index, len);

            tok.kind = Token_String;
            tok.string_value = string;
            break;
        }

        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            int base = 10;
            u64 value = 0;
            while (isalnum(peek_char(P))) {
                int digit = to_digit(peek_char(P));
                if (digit == -1) break;

                value = value * base + digit;
                advance_char(P);
            }

            tok.kind = Token_Integer;
            tok.integer_value = value;
            break;
        }

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case '_': {
            while (isalnum(peek_char(P)) || peek_char(P) == '_') {
                advance_char(P);
            }

            int len = P->stream_index - tok.start.index;
            String name = make_string(P->current_file->content.text + tok.start.index, len);

            auto it = keyword_map.find(name);
            if (it != keyword_map.end()) {
                tok.kind = Token_Name;
                tok.name = atom_create(name);
            } else {
                tok.kind = it->second;
            }
            break;
        }
    }

    tok.end.line = P->current_line;
    tok.end.col = P->current_col;
    tok.end.index = P->stream_index;

    Token last_token = P->current_token;
    P->current_token = tok;

    return last_token;
}

