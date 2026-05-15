#include <assert.h>
#include <unordered_map>

#include "Atom.h"
#include "AST.h"
#include "String.h"
#include "Token.h"
#include "Parser.h"

#define X(K,S) STRZ(S),
String g_token_strings[Token_COUNT] = {
    TOKEN_LIST()
};
#undef X

#define X(K,S) STRZ(S),
String g_operator_strings[Operator_COUNT] = {
    OPERATOR_LIST()
};
#undef X

String string_from_token(TokenKind token) {
    return g_token_strings[token];
}

String string_from_operator(Operator op) {
    return g_operator_strings[op];
}

std::unordered_map<String, TokenKind, StringHasher> keyword_map = [] {
    std::unordered_map<String, TokenKind, StringHasher> m;
    for (int i = Token_KeywordBegin + 1; i < Token_KeywordEnd; i++) {
        TokenKind token = (TokenKind)i;
        String string = string_from_token(token);
        m.insert({string, token});
    }
    return m;
}();

template<typename... Args>
void report_parser_error(Parser *P, std::format_string<Args...> fmt, Args&&... args) {
    std::print("syntax error: {},{}: ", P->current_line, P->current_col);
    std::println(fmt, std::forward<Args>(args)...);
}

void rewind(Parser *P, Token token) {
    P->current_token = token;
    P->current_line = token.end.line;
    P->current_col = token.end.col;
    P->stream_index = token.end.index;
    P->stream = P->current_file->content.text + P->stream_index;
}

AstBlockExpr *parse_block_expr(Parser *P);
Ast *parse_stmt(Parser *P);
Ast *parse_simple_stmt(Parser *P);

Array<Ast*> parse_name_list(Parser *P);

AstProcLit *parse_proc_lit(Parser *P);

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


Token get_token(Parser *P) {
    return P->current_token;
}

Token expect_token(Parser *P, TokenKind token) {
    if (is_token(P, token)) {
        Token token = P->current_token;
        next_token(P);
        return token;
    } else {
        report_parser_error(P, "expected '{}', got '{}'", string_from_token(token), string_from_token(peek_token(P)));
        return {};
    }
}

void init_parser_context(Parser *P, SourceFile *file) {
    P->current_file = file;
    P->current_line = 1;
    P->current_col = 1;
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
        Ast *stmt = parse_stmt(P);
        if (!stmt) break;

        struct_type->members.add(stmt);
    }

    Token close = expect_token(P, Token_CloseBrace);

    struct_type->token = token;
    struct_type->open = open;
    struct_type->close = close;
    return struct_type;
}

AstCase *parse_case_clause(Parser *P) {
    Token token = expect_token(P, Token_Case);

    if (P->block == nullptr || !P->block->is_ifcase) {
        report_parser_error(P, "did not expect case in middle of block");
    }

    Ast *expr = parse_expr(P);

    expect_token(P, Token_Colon);

    AstCase *c = ast_new<AstCase>();
    c->expr = expr;
    c->is_default = expr == nullptr;
    c->token = token;
    return c;
}

AstIfExpr *parse_if_expr(Parser *P) {
    Token token = expect_token(P, Token_If);

    AstIfExpr *if_expr = ast_new<AstIfExpr>();

    Ast *prev_control = P->control_target;
    P->control_target = if_expr;

    int prev_expr_level = P->expr_level;
    P->expr_level = -1;
    Ast *condition = parse_expr(P);

    P->expr_level = prev_expr_level;

    P->control_target = prev_control;

    Ast *then_expr = parse_expr(P);

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
    block->open = open;

    AstBlockExpr *prev_block = P->block;
    P->block = block;

    int prev_expr_level = P->expr_level;
    P->expr_level = 0;

    if (is_token(P, Token_Case)) {
        block->is_ifcase = true;
        if (P->control_target == nullptr || P->control_target->kind != Ast_IfExpr) {
            report_parser_error(P, "case expression in a non-if block");
        }

        AstCase *clause_tail = nullptr;

        while (!is_token(P, Token_CloseBrace)) {
            AstCase *clause = parse_case_clause(P);
            if (!clause) break;

            while (!is_token(P, Token_Case) && !is_token(P, Token_CloseBrace)) {
                Ast *stmt = parse_stmt(P);
                if (!stmt) break;
                clause->statements.add(stmt);
            }

            block->statements.add(clause);

            if (clause_tail) {
                clause_tail->next_clause = clause;
            }
            clause->prev_clause = clause_tail;
            clause->next_clause = nullptr;
            clause_tail = clause;
        }
    } else {
        while (!is_token(P, Token_CloseBrace)) {
            Ast *stmt = parse_stmt(P);
            if (!stmt) break;
            block->statements.add(stmt);
        }
    }

    P->expr_level = prev_expr_level;

    P->block = prev_block;

    Token close = expect_token(P, Token_CloseBrace);
    block->close = close;

    return block;
}

AstStarExpr *parse_star_expr(Parser *P) {
    Token token = expect_token(P, Token_Star);
    Ast *elem = parse_type(P);
    AstStarExpr *expr = ast_new<AstStarExpr>();
    expr->elem = elem;
    expr->token = token;
    return expr;
}

Ast *parse_type(Parser *P) {
    bool prev_allow_type = P->allow_type;
    P->allow_type = true;

    Ast *expr = parse_operand(P);

    P->allow_type = prev_allow_type;

    return expr;
}

AstProcType *parse_proc_type(Parser *P) {
    Token open = expect_token(P, Token_OpenParen);

    AstProcType *proc_type = ast_new<AstProcType>();

    bool named = false;

    while (!is_token(P, Token_CloseParen)) {
        Array<Ast*> lhs = parse_expr_list(P);
        Ast *type_defn = nullptr;
        Array<Ast*> rhs;

        if (match_token(P, Token_Colon)) {
            named = true;
            type_defn = parse_type(P);

            if (type_defn) {
                if (match_token(P, Token_Assign)) {
                    rhs = parse_expr_list(P);
                    if (rhs.count == 0) {
                        report_parser_error(P, "missing expression after '='");
                    }
                }
            }
        } else {
            report_parser_error(P, "expected ':' after field list, got {}", string_from_token(peek_token(P)));
        }

        AstParam *param = ast_new<AstParam>();
        param->lhs = lhs;
        param->rhs = rhs;
        param->type_defn = type_defn;

        proc_type->params.add(param);

        if (!match_token(P, Token_Comma)) break;
    }

    Token close = expect_token(P, Token_CloseParen);

    Ast *ret_type = nullptr;
    if (match_token(P, Token_Arrow)) {
        ret_type = parse_type(P);
    }

    proc_type->return_type = ret_type;

    return proc_type;
}

AstProcLit *ast_proc_lit(AstProcType *type, AstBlockExpr *body) {
    AstProcLit *proc_lit = ast_new<AstProcLit>();
    proc_lit->proc_type = type;
    proc_lit->body = body;
    return proc_lit;
}

AstParenExpr *parse_paren_expr(Parser *P) {
    Token open = expect_token(P, Token_OpenParen);
    Ast *elem = parse_expr(P);
    Token close = expect_token(P, Token_CloseParen);

    AstParenExpr *paren = ast_new<AstParenExpr>();
    paren->expr = elem;
    paren->open = open;
    paren->close = close;
    return paren;
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

        case Token_Star:
            return parse_star_expr(P);

        case Token_OpenBracket: {
            Token open = expect_token(P, Token_OpenBracket);

            Ast *size = parse_expr(P);

            Token close = expect_token(P, Token_CloseBracket);

            Ast *elem = parse_type(P);

            AstArrayType *type = ast_new<AstArrayType>();
            type->size = size;
            type->elem = elem;
            type->open = open;
            type->close = close;
            return type;
        }

        case Token_OpenParen: {
            if (P->allow_type) {
                return parse_proc_type(P);
            }

            Token open = expect_token(P, Token_OpenParen);
            bool is_type = false;

            if (is_token(P, Token_CloseParen)) {
                is_type = true;
                rewind(P, open);
            }

            if (!is_type) {
                Ast *expr = parse_expr(P);
                if (is_token(P, Token_Colon) || is_token(P, Token_Comma)) {
                    is_type = true;
                }
                rewind(P, open);
            }

            AstProcType *proc_type = nullptr;

            if (is_type) {
                proc_type = parse_proc_type(P);

                if (is_token(P, Token_OpenBrace)) {
                    AstBlockExpr *body = parse_block_expr(P);
                    return ast_proc_lit(proc_type, body);
                } else if (is_token(P, Token_UnInit)) {
                    return ast_proc_lit(proc_type, nullptr);
                }

                return proc_type;
            }

            return parse_paren_expr(P);
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

        if (!match_token(P, Token_Comma)) {
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
                if (P->expr_level >= 0) {
                    operand = parse_compound_literal(P, operand);
                } else {
                    loop = false;
                }
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

Operator get_binary_op(TokenKind token) {
    switch (token) {
        default:
            return Operator_Nil;
        case Token_Plus:
            return Operator_Add;
        case Token_Minus:
            return Operator_Sub;
        case Token_Star:
            return Operator_Mult;
        case Token_Slash:
            return Operator_Div;
        case Token_Percent:
            return Operator_Mod;
        case Token_Equal:
            return Operator_Equal;
        case Token_NotEqual:
            return Operator_NotEqual;
        case Token_Less:
            return Operator_Less;
        case Token_Greater:
            return Operator_Greater;
        case Token_LessEqual:
            return Operator_LessEqual;
        case Token_GreaterEqual:
            return Operator_GreaterEqual;
        case Token_And:
            return Operator_And;
        case Token_Or:
            return Operator_Or;
        case Token_Bar:
            return Operator_BitwiseOr;
        case Token_Ampersand:
            return Operator_BitwiseAnd;
        case Token_LeftShift:
            return Operator_LeftShift;
        case Token_RightShift:
            return Operator_RightShift;
    }
}

int get_op_prec(Operator op) {
    switch (op) {
        default:
            return -1;

        case Operator_Mult:
        case Operator_Div:
        case Operator_Mod:
            return 11000;

        case Operator_Add:
        case Operator_Sub:
            return 10000;

        case Operator_Less:
        case Operator_Greater:
        case Operator_LessEqual:
        case Operator_GreaterEqual:
            return 9000;

        case Operator_Equal:
        case Operator_NotEqual:
            return 8000;

        case Operator_BitwiseAnd:
            return 7000;
        case Operator_Xor:
            return 6000;
        case Operator_BitwiseOr:
            return 5000;

        case Operator_And:
            return 4000;
        case Operator_Or:
            return 3000;
        case Operator_LeftShift:
        case Operator_RightShift:
            return 2000;
    }
}

AstBinaryExpr *ast_binary_expr(Token token, Operator op, Ast *lhs, Ast *rhs) {
    AstBinaryExpr *expr = ast_new<AstBinaryExpr>();
    expr->token = token;
    expr->op = op;
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}

Ast *parse_binary_expr(Parser *P, Ast *lhs, int current_prec) {
    for (;;) {
        Token op_token = get_token(P);
        Operator op = get_binary_op(op_token.kind);

        int op_prec = get_op_prec(op);

        if (op_prec < current_prec) {
            return lhs;
        }

        next_token(P);

        Ast *rhs = parse_unary_expr(P);
        if (rhs == nullptr) {
            report_parser_error(P, "expected expression after {}", string_from_operator(op));
            return nullptr;
        }

        Token next_tok = get_token(P);
        Operator next_op = get_binary_op(next_tok.kind);
        int next_prec = get_op_prec(next_op);

        if (op_prec < next_prec) {
            rhs = parse_binary_expr(P, rhs, op_prec + 1);

            if (rhs == nullptr) {
                rhs = ast_binary_expr(op_token, op, lhs, nullptr);
                return rhs;
            }
        }

        lhs = ast_binary_expr(op_token, op, lhs, rhs);
    }

    return lhs;
}

Ast *parse_expr(Parser *P) {
    Ast *lhs = parse_unary_expr(P);
    return parse_binary_expr(P, lhs, 0);
}

Ast *parse_simple_stmt(Parser *P) {
    Array<Ast*> lhs = parse_expr_list(P);

    if (lhs.count == 0) return nullptr;

    Array<Ast*> rhs;
    if (match_token(P, Token_Colon)) {
        Ast *type_defn = nullptr;
        bool is_constant = false;

        if (match_token(P, Token_Colon)) {
            // compile-time constant
            rhs = parse_expr_list(P);
            is_constant = true;
        } else {
            // non compile-time constant
            type_defn = parse_type(P);

            if (match_token(P, Token_Assign)) {
                rhs = parse_expr_list(P);
            }
        }

        AstValueDecl *vd = ast_new<AstValueDecl>();
        vd->lhs = lhs;
        vd->rhs = rhs;
        vd->type_defn = type_defn;
        vd->is_constant = is_constant;

        if (!is_constant) {
            expect_token(P, Token_Semicolon);
        }
        return vd;
    } else if (match_token(P, Token_Assign)) {
        rhs = parse_expr_list(P);
        AstAssign *assign = ast_new<AstAssign>();
        assign->lhs = lhs;
        assign->rhs = rhs;
        expect_token(P, Token_Semicolon);
        return assign;
    } else {
        Ast *expr = lhs[0];
        // NOTE: Trailing block-ish expression
        if (is_token(P, Token_Case) || is_token(P, Token_CloseBrace)) {
            P->block->trailing = expr;
            return expr;
        } else {
            if (expr->kind != Ast_IfExpr) {
                expect_token(P, Token_Semicolon);
            }

            AstExprStmt *stmt = ast_new<AstExprStmt>();
            stmt->expr = expr;
            return stmt;
        }
    }
}

Ast *parse_stmt(Parser *P) {
    Ast *stmt = nullptr;

    switch (peek_token(P)) {
        default:
            stmt = parse_simple_stmt(P);
            break;

        case Token_Semicolon: {
            Token token = expect_token(P, Token_Semicolon);
            AstEmptyStmt *empty = ast_new<AstEmptyStmt>();
            empty->token = token;
            stmt = empty;
            break;
        }

        case Token_While: {
            Token token = expect_token(P, Token_While);

            AstWhile *while_stmt = ast_new<AstWhile>();

            int prev_expr_level = P->expr_level;
            P->expr_level = -1;
            Ast *condition = parse_expr(P);

            P->expr_level = prev_expr_level;


            while_stmt->condition = condition;

            Ast *prev_control = P->control_target;
            P->control_target = while_stmt;

            AstBlockExpr *block = parse_block_expr(P);
            while_stmt->block = block;

            P->control_target = prev_control;

            stmt = while_stmt;
            break;
        }

        case Token_Do: {
            Token token = expect_token(P, Token_Do);

            AstDo *do_stmt = ast_new<AstDo>();

            Ast *prev_control = P->control_target;
            P->control_target = do_stmt;

            AstBlockExpr *block = parse_block_expr(P);

            expect_token(P, Token_While);
            Ast *condition = parse_expr(P);

            P->control_target = prev_control;

            do_stmt->condition = condition;
            do_stmt->block = block;
            stmt = do_stmt;
            break;
        }

        case Token_For: {
            Token token = expect_token(P, Token_For);

            AstFor *for_stmt = ast_new<AstFor>();

            int prev_expr_level = P->expr_level;
            P->expr_level = -1;
            Ast *condition = parse_expr(P);

            P->expr_level = prev_expr_level;

            for_stmt->condition = condition;

            Ast *prev_control = P->control_target;
            P->control_target = for_stmt;

            AstBlockExpr *block = parse_block_expr(P);

            P->control_target = prev_control;

            for_stmt->block = block;
            for_stmt->token = token;
            stmt = for_stmt;
            break;
        }

        case Token_Break: {
            Token token = expect_token(P, Token_Break);
            AstBreak *break_stmt = ast_new<AstBreak>();
            break_stmt->token = token;
            stmt = break_stmt;
            break;
        }

        case Token_Continue: {
            Token token = expect_token(P, Token_Continue);
            AstContinue *continue_stmt = ast_new<AstContinue>();
            continue_stmt->token = token;
            stmt = continue_stmt;
            break;
        }

        case Token_Fallthrough: {
            Token token = expect_token(P, Token_Fallthrough);
            AstFallthrough *fallthrough_stmt = ast_new<AstFallthrough>();
            fallthrough_stmt->token = token;
            stmt = fallthrough_stmt;
            break;
        }

        case Token_Return: {
            Token token = expect_token(P, Token_Return);
            AstReturn *ret = ast_new<AstReturn>();
            Ast *expr = parse_expr(P);
            ret->expr = expr;
            ret->token = token;
            stmt = ret;
            break;
        }

        case Token_Case:
            report_parser_error(P, "case clause unexpected");
            break;

        case Token_Else:
            report_parser_error(P, "illegal else without matching if");
            break;
    }
    return stmt;
}

void advance_char(Parser *P) {
    if (P->stream_index == P->current_file->content.len) {
        return;
    }

    if (peek_char(P) == '\r') {
        P->stream_index++;
        if (peek_char(P) == '\n') {
            P->stream_index++;
        }
        P->current_line++;
        P->current_col = 1;
    } else if (peek_char(P) == '\n') {
        P->stream_index++;
        P->current_line++;
        P->current_col = 1;
    } else {
        P->stream_index++;
        P->current_col++;
    }

    P->stream = P->current_file->content.text + P->stream_index;
}

Token next_token(Parser *P) {
    Token tok = {};

scan_begin:
    tok.start.line  = P->current_line;
    tok.start.col   = P->current_col;
    tok.start.index = P->stream_index;

    #define TOKCASE(C,T) case C: advance_char(P); tok.kind = T; break

    switch (peek_char(P)) {
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
            } else if (peek_char(P) == '-' && peek_next_char(P) == '-') {
                advance_char(P);
                advance_char(P);
                tok.kind = Token_UnInit;
            } else {
                tok.kind = Token_Minus;
            }
            break;

        case '+':
            advance_char(P);
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_PlusAssign;
            } else {
                tok.kind = Token_Plus;
            }
            break;

        case '/':
            advance_char(P);
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_DivAssign;
            } else if (peek_char(P) == '/') {
                advance_line(P);
                goto scan_begin;
            } else {
                tok.kind = Token_Slash;
            }
            break;

        case '=':
            advance_char(P);
            if (peek_char(P) == '=') {
                advance_char(P);
                tok.kind = Token_Equal;
            } else {
                tok.kind = Token_Assign;
            }
            break;

        case '|':
            advance_char(P);
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
            advance_char(P);
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
            advance_char(P);
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
            advance_char(P);
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
            advance_char(P);
            while (peek_char(P) != '"') {
                advance_char(P);
            }
            advance_char(P);

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
                tok.kind = it->second;
            } else {
                tok.kind = Token_Name;
                tok.name = atom_create(name);
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
