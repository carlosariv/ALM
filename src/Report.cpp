#include "Report.h"

Token ast_start_token(Ast *node) {
#define AST_X(Name,Type) Type *Name = static_cast<Type*>(node);

    switch (node->kind) {
        default: 
        case Ast_File:
            break;

        case Ast_ValueDecl: {
            AST_X(vd, AstValueDecl);
            return ast_start_token(vd->lhs[0]);
        }

        case Ast_Assign: {
            AST_X(as, AstAssign);
            return as->lhs[0];
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
            return ast_start_token(cs->token);
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

        case Ast_Name: {
            AST_X(name, AstName);
            return name->token;
        }
        case Ast_LiteralExpr: {
            AST_X(le, LiteralExpr);
            return le->token;
        }
        case Ast_UnaryExpr: {
            AST_X(ue, AstUnaryExpr);
            return ue->token;
        }
        case Ast_BinaryExpr: {
            AST_X(be, AstBinaryExpr);
            return ast_start_token(be->lhs);
        }
        case Ast_SelectorExpr: {
            AST_X(se, AstSelectorExpr);
            return se->token;
        }
        case Ast_SubscriptExpr: {
            AST_X(se, AstSubscriptExpr);
            return se->open;
        }
        case Ast_CallExpr: {
            AST_X(ce, AstCallExpr);
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
            AST_X(cl, CompoundLiteral);
            return cl->open;
        }
        case Ast_IfExpr: {
            AST_X(ie, IfExpr);
            return ie->token;
        }
        case Ast_StarExpr: {
            AST_X(se, StarExpr);
            return se->token;
        }

        case Ast_ProcType: {
            AST_X(pt, ProcTypeDefn);
            return pt->open;
        }
        case Ast_ProcLit: {
            AST_X(pl, ProcLit);
            return ast_start_token(pl->proc_type);
        }
        case Ast_Param: {
            AST_X(param, AstParam);
            return ast_start_token(param->lhs);
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
    }
    return {};
}


Token ast_end_token(Ast *node) {
    return {};
}
