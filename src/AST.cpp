#include <cassert>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <filesystem>
#include <print>

#include "Token.h"
#include "Ast.h"
#include "Atom.h"

const char *g_ast_strings[Ast_COUNT];

int tab_width = 0;

void *ast_alloc(int bytes) {
    void *mem = malloc(bytes);
    return mem;
}

void tab_begin() {
    tab_width += 1;
}

void tab_end() {
    assert(tab_width > 0);
    tab_width -= 1;
}

// std::ostream ast_os;
template<typename... Args>
void ast_out(std::format_string<Args...> fmt, Args&&... args) {
    std::print(/*ast_os, //ostream*/ fmt, std::forward<Args>(args)...); // Print the rest and a newline
}

template<typename... Args>
void ast_out_tab(std::format_string<Args...> fmt, Args&&... args) {
    for (int i = 0; i < tab_width; i++) {
        std::print("\t");
    }
    std::print(/*ast_os, //ostream*/ fmt, std::forward<Args>(args)...); // Print the rest and a newline
}

void ast_print(Ast *node) {
    if (!node) return;

    switch (node->kind) {
        case Ast_Unknown:
            break;

        case Ast_File: {
            AstFile *ast_file = static_cast<AstFile*>(node);
            ast_out("(file\n");
            tab_begin();
            for (Ast *member : ast_file->decls) {
                ast_print(member);
            }
            tab_end();
            ast_out(")");
            break;
        }

        case Ast_Name: {
            AstName *name = static_cast<AstName*>(node);
            ast_out("{}", (char *)(name->name->text));
            break;
        }

        case Ast_ProcLit: {
            AstProcLit *proc_lit = static_cast<AstProcLit*>(node);
            ast_out("(proc (");
            for (Ast *param : proc_lit->params) {
                ast_print(param);
            }
            ast_out(")\n");

            if (proc_lit->return_type) {
                ast_out("-> (");
                ast_print(proc_lit->return_type);
                ast_out(")");
            }
            tab_begin();
            ast_print(proc_lit->body);
            tab_end();
            ast_out(")\n");
            break;
        }

        case Ast_ValueDecl: {
            AstValueDecl *decl = static_cast<AstValueDecl*>(node);
            ast_out("(decl (");
            for (Ast *name : decl->lhs) {
                ast_print(name);
                ast_out(" ");
            }
            ast_out(")\n");

            ast_out("(");
            for (Ast *expr : decl->rhs) {
                ast_print(expr);
                ast_out("\n");
            }
            ast_out(")");

            ast_out(")\n");
            break;
        }

        case Ast_Param: {
            AstParam *param = static_cast<AstParam*>(node);
            ast_out("(: ");
            //ast_print(param->name);
            ast_print(param->type_defn);
            ast_out(" )");
            break;
        }

        case Ast_LiteralExpr: {
            AstLiteralExpr *literal = static_cast<AstLiteralExpr*>(node);
            switch (literal->literal_kind) {
                case Literal_Integer:
                    ast_out("{}", literal->integer_value);
                    break;
                case Literal_Floating:
                    ast_out("{}", literal->float_value);
                    break;
                case Literal_String:
                    ast_out("\"{}\"", (char *)literal->string_value.text);
                    break;
            }
            break;
        }

        case Ast_UnaryExpr: {
            AstUnaryExpr *unary = static_cast<AstUnaryExpr*>(node);
            ast_out("({} ", string_from_operator(unary->op));
            ast_print(unary->operand);
            ast_out(")");
            break;
        }

        case Ast_BinaryExpr: {
            AstBinaryExpr *binary = static_cast<AstBinaryExpr*>(node);
            ast_out("({} ", string_from_operator(binary->op));
            ast_print(binary->lhs);
            ast_out(" ");
            ast_print(binary->rhs);
            ast_out(")");
            break;
        }

        case Ast_IndexExpr: {
            AstIndexExpr *index = static_cast<AstIndexExpr*>(node);
            break;
        }

        case Ast_IfExpr: {
            AstIfExpr *if_expr = static_cast<AstIfExpr*>(node);
            if (if_expr->condition) {
                ast_out("(if\n");
                ast_out("(");
                ast_print(if_expr->condition);
                ast_out(")");
            } else {
                ast_out("(else\n");
            }

            ast_print(if_expr->then_expr);

            ast_print(if_expr->else_if);
            break;
        }

        case Ast_BlockExpr: {
            AstBlockExpr *block = static_cast<AstBlockExpr*>(node);
            ast_out("(block \n");
            tab_begin();
            for (Ast *stmt : block->statements) {
                ast_print(stmt);
            }
            tab_end();
            ast_out(")\n");
            break;
        }

        case Ast_ExprStmt: {
            AstExprStmt *stmt = static_cast<AstExprStmt*>(node);
            ast_out("(expr \n");
            ast_print(stmt->expr);
            ast_out(")\n");
            break;
        }

        case Ast_Assign: {
            AstAssign *assign = static_cast<AstAssign*>(node);
            ast_out("(= \n");
            for (Ast *expr : assign->lhs) {
                ast_print(expr);
                ast_out(" ");
            }

            ast_out("(");
            for (Ast *expr : assign->rhs) {
                ast_print(expr);
                ast_out(" ");
            }
            ast_out(")");
            ast_out(")\n");
            break;
        }

        case Ast_Return: {
            AstReturn *ret = static_cast<AstReturn*>(node);
            ast_out("(return ");
            ast_print(ret->expr);
            ast_out(")\n");
            break;
        }

        case Ast_PointerType: {
            break;
        }
        case Ast_ArrayType: {
            break;
        }
        case Ast_StructType: {
            break;
        }
        case Ast_UnionType: {
            break;
        }
    }
}
