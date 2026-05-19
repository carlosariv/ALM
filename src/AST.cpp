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
        case Ast_Error:
        case Ast_EmptyStmt:
        case Ast_COUNT:
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

        case Ast_Ident: {
            Ident *name = static_cast<Ident*>(node);
            ast_out("{}", (char *)(name->name->text));
            break;
        }

        case Ast_ProcType: {
            ProcTypeDefn *proc_type = static_cast<ProcTypeDefn*>(node);
            ast_out("(");
            for (Param *param : proc_type->params) {
                ast_print(param);
            }
            ast_out(")");

            if (proc_type->return_type) {
                ast_out("-> (");
                ast_print(proc_type->return_type);
                ast_out(")");
            }

            break;
        }

        case Ast_ProcLit: {
            ProcLit *proc_lit = static_cast<ProcLit*>(node);
            ast_print(proc_lit->proc_type);

            tab_begin();
            ast_print(proc_lit->body);
            tab_end();
            ast_out(")\n");
            break;
        }

        case Ast_ValueDecl: {
            ValueDecl *decl = static_cast<ValueDecl*>(node);
            ast_out("(decl ");
            for (Ast *name : decl->lhs) {
                ast_print(name);
                ast_out(" ");
            }

            if (decl->type_defn) {
                ast_print(decl->type_defn);
            }

            if (decl->rhs.count > 0) {
                ast_out("(");
                for (Ast *expr : decl->rhs) {
                    ast_print(expr);
                    ast_out("\n");
                }
                ast_out(")");
            }

            ast_out(")\n");
            break;
        }

        case Ast_CompoundLiteral: {
            CompoundLiteralExpr *cl = static_cast<CompoundLiteralExpr*>(node);
            ast_out("(compound ");
            for (Ast *init :  cl->initializer_list) {
                ast_print(init);
            }
            ast_out(")");
            break;
        }

        case Ast_Param: {
            Param *param = static_cast<Param*>(node);
            ast_out("(");
            for (Ast *name : param->lhs) {
                ast_print(name);
                ast_out(",");
            }

            if (param->type_defn) {
                ast_out(":");
                ast_print(param->type_defn);
            }

            if (param->rhs.count > 0) {
                ast_out("= (");
                for (Ast *e : param->rhs) {
                    ast_print(e);
                }
                ast_out(")");
            }

            ast_out(")");
            break;
        }

        case Ast_LiteralExpr: {
            LiteralExpr *literal = static_cast<LiteralExpr*>(node);
            switch (literal->literal_kind) {
                case Literal_Integer:
                    ast_out("{}", literal->integer_value);
                    break;
                case Literal_Floating:
                    ast_out("{}", literal->float_value);
                    break;
                case Literal_String:
                    ast_out("\"{}\"", literal->string_value);
                    break;
            }
            break;
        }

        case Ast_UnaryExpr: {
            UnaryExpr *unary = static_cast<UnaryExpr*>(node);
            ast_out("({} ", string_from_operator(unary->op));
            ast_print(unary->operand);
            ast_out(")");
            break;
        }

        case Ast_BinaryExpr: {
            BinaryExpr *binary = static_cast<BinaryExpr*>(node);
            ast_out("({} ", string_from_operator(binary->op));
            ast_print(binary->lhs);
            ast_out(" ");
            ast_print(binary->rhs);
            ast_out(")");
            break;
        }

        case Ast_SubscriptExpr: {
            SubscriptExpr *subscript = static_cast<SubscriptExpr*>(node);
            ast_out("(subscript ");
            ast_print(subscript->operand);
            ast_out(" ");
            ast_print(subscript->value);
            ast_out(")");
            break;
        }

        case Ast_SelectorExpr: {
            SelectorExpr *se = static_cast<SelectorExpr*>(node);
            ast_out("(. ");
            ast_print(se->operand);
            ast_out(" ");
            ast_print(se->name);
            ast_out(")");
            break;
        }

        case Ast_CallExpr: {
            CallExpr *call = static_cast<CallExpr*>(node);
            ast_out("(call ");
            ast_print(call->operand);

            if (call->arguments.count > 0) {
                ast_out("(");
                for (Ast *arg : call->arguments) {
                    ast_print(arg);
                    ast_out(" ");
                }
                ast_out(")");
            }
            ast_out(")");
            break;
        }

        case Ast_ParenExpr: {
            ParenExpr *p = static_cast<ParenExpr*>(node);
            ast_out("(");
            ast_print(p->expr);
            ast_out(")");
            break;
        }

        case Ast_IfExpr: {
            IfExpr *if_expr = static_cast<IfExpr*>(node);
            if (if_expr->condition) {
                ast_out("(if ");
                ast_out("(");
                ast_print(if_expr->condition);
                ast_out(")");
            } else {
                ast_out("(else\n");
            }

            ast_print(if_expr->then_expr);

            if (if_expr->else_if) {
                ast_print(if_expr->else_if);
            }
            break;
        }

        case Ast_BlockExpr: {
            BlockExpr *block = static_cast<BlockExpr*>(node);
            ast_out("(block \n");
            tab_begin();
            for (Ast *stmt : block->statements) {
                ast_print(stmt);
                ast_out("\n");
            }
            tab_end();
            ast_out(")\n");
            break;
        }

        case Ast_ExprStmt: {
            ExprStmt *stmt = static_cast<ExprStmt*>(node);
            ast_out("(expr \n");
            ast_print(stmt->expr);
            ast_out(")\n");
            break;
        }

        case Ast_Assign: {
            AssignStmt *assign = static_cast<AssignStmt*>(node);
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
            ReturnStmt *ret = static_cast<ReturnStmt*>(node);
            ast_out("(return");
            if (ret->expr) {
                ast_out(" ");
                ast_print(ret->expr);
            }
            ast_out(")");
            break;
        }

        case Ast_StarExpr: {
            StarExpr *star = static_cast<StarExpr*>(node);
            ast_out("(* ");
            ast_print(star->elem);
            ast_out(")");
            break;
        }

        case Ast_EnumType: {
            EnumTypeDefn *et = static_cast<EnumTypeDefn*>(node);
            ast_out("(enum \n");
            for (Ast *m : et->members) {
                ast_print(m);
                ast_out("\n");
            }
            ast_out(")");
            break;
        }

        case Ast_Enumerator: {
            Enumerator *en = static_cast<Enumerator*>(node);
            ast_out("(enumerator ");
            ast_print(en->name);
            if (en->value) {
                ast_out(" ");
                ast_print(en->value);
            }
            ast_out(")");
            break;
        }

        case Ast_ArrayType: {
            ArrayTypeDefn *type = static_cast<ArrayTypeDefn*>(node);
            ast_out("[");
            if (type->dynamic) {
                ast_out("..");
            } else if (type->size) {
                ast_print(type->size);
            }
            ast_out("]");
            ast_print(type->elem);
            break;
        }

        case Ast_StructType: {
            StructTypeDefn *type = static_cast<StructTypeDefn*>(node);
            ast_out("struct (");
            for (Ast *member : type->members) {
                ast_print(member);
            }
            ast_out(")");
            break;
        }

        case Ast_UnionType: {
            UnionTypeDefn *type = static_cast<UnionTypeDefn*>(node);
            ast_out("struct (");
            for (Ast *member : type->members) {
                ast_print(member);
            }
            ast_out(")");
            break;
        }

        case Ast_Do: {
            DoStmt *d = static_cast<DoStmt*>(node);
            ast_out("(do\n");
            ast_print(d->block);
            ast_out(") (");
            ast_print(d->condition);
            ast_out(")\n");
            break;
        }

        case Ast_While: {
            WhileStmt *d = static_cast<WhileStmt*>(node);
            ast_out("(while\n");
            ast_out("(");
            ast_print(d->condition);
            ast_out(")\n");
            ast_print(d->block);
            break;
        }

        case Ast_For: {
            ForStmt *f = static_cast<ForStmt*>(node);
            ast_out("(for\n");
            ast_out("(");
            ast_print(f->condition);
            ast_out(")\n");
            ast_print(f->block);
            break;
        }

        case Ast_Case: {
            CaseExpr *c = static_cast<CaseExpr*>(node);
            ast_out("(case ");
            if (c->expr) {
                ast_out("(");
                ast_print(c->expr);
                ast_out(")");
            }
            ast_out("\n(");
            for (Ast *stmt : c->statements) {
                ast_print(stmt);
            }
            ast_out(")");
            break;
        }

        case Ast_Continue: {
            ast_out("(continue)");
            break;
        }
        case Ast_Break: {
            ast_out("(break)");
            break;
        }
        case Ast_Fallthrough: {
            ast_out("(fallthrough)");
            break;
        }
    }
}


