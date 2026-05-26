#include "Report.h"

#define AST_X(Name,Type) Type *Name = static_cast<Type*>(node);

Token ast_start_token(Ast *node) {
    switch (node->kind) {
        case Ast_ValueDecl: {
            AST_X(vd, ValueDecl);
            return ast_start_token(vd->names[0]);
        }

        case Ast_Assign: {
            AST_X(as, AssignStmt);
            return ast_start_token(as->lhs[0]);
        }

        case Ast_ExprStmt: {
            AST_X(es, ExprStmt);
            return ast_start_token(es->expr);
        }
        case Ast_EmptyStmt: {
            AST_X(es, EmptyStmt);
            return es->token;
        }
        case Ast_Break: {
            AST_X(br, BreakStmt);
            return br->token;
        }
        case Ast_Continue: {
            AST_X(c, ContinueStmt);
            return c->token;
        }
        case Ast_Fallthrough: {
            AST_X(ft, FallthroughStmt);
            return ft->token;
        }
        case Ast_Return: {
            AST_X(ret, ReturnStmt);
            return ret->token;
        }
        case Ast_Case: {
            AST_X(cs, CaseExpr);
            return cs->token;
        }
        case Ast_Do: {
            AST_X(ds, DoStmt);
            return ds->token;
        }
        case Ast_While: {
            AST_X(ws, WhileStmt);
            return ws->token;
        }
        case Ast_For: {
            AST_X(fs, ForStmt);
            return fs->token;
        }

        case Ast_Ident: {
            AST_X(name, Ident);
            return name->token;
        }
        case Ast_LiteralExpr: {
            AST_X(le, LiteralExpr);
            return le->token;
        }
        case Ast_UnaryExpr: {
            AST_X(ue, UnaryExpr);
            return ue->token;
        }
        case Ast_BinaryExpr: {
            AST_X(be, BinaryExpr);
            return ast_start_token(be->lhs);
        }
        case Ast_SelectorExpr: {
            AST_X(se, SelectorExpr);
            return se->token;
        }
        case Ast_CallExpr: {
            AST_X(ce, CallExpr);
            return ce->open;
        }
        case Ast_ParenExpr: {
            AST_X(pe, ParenExpr);
            return pe->open;
        }
        case Ast_BlockExpr: {
            AST_X(be, BlockExpr);
            return be->open;
        }
        case Ast_CompoundLiteral: {
            AST_X(cl, CompoundLiteralExpr);
            return cl->open;
        }
        case Ast_ArrayExpr: {
            AST_X(ae, ArrayExpr);
            if (ae->operand) {
                return ast_start_token(ae->operand);
            }
            return ae->open;
        }
        case Ast_IfExpr: {
            AST_X(ie, IfExpr);
            return ie->token;
        }
        case Ast_IfCaseExpr: {
            AST_X(ice, IfCaseExpr);
            return ice->token;
        }
        case Ast_StarExpr: {
            AST_X(se, StarExpr);
            return se->token;
        }

        case Ast_DerefExpr: {
            AST_X(de, DerefExpr);
            return ast_start_token(de->operand);
        }

        case Ast_Param: {
            AST_X(param, Param);
            if (param->names.count > 0) {
                return ast_start_token(param->names[0]);
            }
            return ast_start_token(param->type_defn);
        }

        case Ast_ProcType: {
            AST_X(pt, ProcTypeDefn);
            return pt->open;
        }
        case Ast_ProcLit: {
            AST_X(pl, ProcLit);
            return ast_start_token(pl->proc_type);
        }
        case Ast_ArrayType: {
            AST_X(at, ArrayTypeDefn);
            return at->open;
        }
        case Ast_StructType: {
            AST_X(st, StructTypeDefn);
            return st->token;
        }
        case Ast_UnionType: {
            AST_X(ut, UnionTypeDefn);
            return ut->token;
        }
        case Ast_EnumType: {
            AST_X(et, EnumTypeDefn);
            return et->token;
        }
        case Ast_Enumerator: {
            AST_X(enu, Enumerator);
            return ast_start_token(enu->name);
        }

        case Ast_Unknown:
        case Ast_Error:
        case Ast_File:
        case Ast_COUNT:
            break;
    }
    return {};
}

Token ast_end_token(Ast *node) {
    switch (node->kind) {
        case Ast_ValueDecl: {
            AST_X(vd, ValueDecl);
            if (!vd->values.is_empty()) {
                return ast_end_token(vd->values[vd->values.count-1]);
            } else {
                return ast_end_token(vd->type_defn);
            }
        }

        case Ast_Assign: {
            AST_X(as, AssignStmt);
            return ast_end_token(as->rhs[as->rhs.count-1]);
        }

        case Ast_ExprStmt: {
            AST_X(es, ExprStmt);
            return ast_end_token(es->expr);
        }
        case Ast_EmptyStmt: {
            AST_X(es, EmptyStmt);
            return es->token;
        }
        case Ast_Break: {
            AST_X(br, BreakStmt);
            return br->token;
        }
        case Ast_Continue: {
            AST_X(c, ContinueStmt);
            return c->token;
        }
        case Ast_Fallthrough: {
            AST_X(ft, FallthroughStmt);
            return ft->token;
        }
        case Ast_Return: {
            AST_X(ret, ReturnStmt);
            if (ret->results.count > 0) {
                return ast_end_token(ret->results[ret->results.count - 1]);
            }
            return ret->token;
        }
        case Ast_Case: {
            AST_X(cs, CaseExpr);
            if (!cs->statements.is_empty()) {
                return ast_end_token(cs->statements[cs->statements.count-1]);
            }
            if (cs->expr) {
                return ast_end_token(cs->expr);
            }
            return cs->token;
        }
        case Ast_Do: {
            AST_X(ds, DoStmt);
            return ast_end_token(ds->block);
        }
        case Ast_While: {
            AST_X(ws, WhileStmt);
            return ast_end_token(ws->block);
        }
        case Ast_For: {
            AST_X(fs, ForStmt);
            return ast_end_token(fs->block);
        }

        case Ast_Ident: {
            AST_X(name, Ident);
            return name->token;
        }
        case Ast_LiteralExpr: {
            AST_X(le, LiteralExpr);
            return le->token;
        }
        case Ast_UnaryExpr: {
            AST_X(ue, UnaryExpr);
            return ast_end_token(ue->operand);
        }
        case Ast_BinaryExpr: {
            AST_X(be, BinaryExpr);
            return ast_end_token(be->rhs);
        }
        case Ast_SelectorExpr: {
            AST_X(se, SelectorExpr);
            return ast_end_token(se->name);
        }
        case Ast_CallExpr: {
            AST_X(ce, CallExpr);
            return ce->close;
        }
        case Ast_ParenExpr: {
            AST_X(pe, ParenExpr);
            return pe->close;
        }
        case Ast_BlockExpr: {
            AST_X(be, BlockExpr);
            return be->close;
        }
        case Ast_CompoundLiteral: {
            AST_X(cl, CompoundLiteralExpr);
            return cl->close;
        }
        case Ast_ArrayExpr: {
            AST_X(ae, ArrayExpr);
            return ae->close;
        }
        case Ast_IfExpr: {
            AST_X(ie, IfExpr);
            return ast_end_token(ie->then_expr);
        }
        case Ast_IfCaseExpr: {
            AST_X(ice, IfCaseExpr);
            return ast_end_token(ice->block);
        }
        case Ast_StarExpr: {
            AST_X(se, StarExpr);
            return ast_end_token(se->elem);
        }
        case Ast_DerefExpr: {
            AST_X(de, DerefExpr);
            return de->token;
        }

        case Ast_Param: {
            AST_X(param, Param);
            if (param->default_value) {
                return ast_end_token(param->default_value);
            } else {
                return ast_end_token(param->type_defn);
            }
        }

        case Ast_ProcType: {
            AST_X(pt, ProcTypeDefn);
            if (pt->results.count > 0) {
                return ast_end_token(pt->results[pt->results.count-1]);
            }
            return pt->close;
        }
        case Ast_ProcLit: {
            AST_X(pl, ProcLit);
            if (pl->body) {
                return ast_end_token(pl->body);
            }
            return ast_end_token(pl->proc_type);
        }
        case Ast_ArrayType: {
            AST_X(at, ArrayTypeDefn);
            return at->close;
        }
        case Ast_StructType: {
            AST_X(st, StructTypeDefn);
            return st->close;
        }
        case Ast_UnionType: {
            AST_X(ut, UnionTypeDefn);
            return ut->close;
        }
        case Ast_EnumType: {
            AST_X(et, EnumTypeDefn);
            return et->close;
        }
        case Ast_Enumerator: {
            AST_X(enu, Enumerator);
            if (enu->value) {
                return ast_end_token(enu->value);
            }
            return ast_end_token(enu->name);
        }
        case Ast_Unknown:
        case Ast_Error:
        case Ast_File:
        case Ast_COUNT:
            break;
    }
    return {};
}
