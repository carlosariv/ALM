#include "Resolve.h"

void resolve_expr(Ast *expr) {
    switch (expr->kind) {
    }
}

void resolve_value_decl(AstValueDecl *vd) {
    for (Ast *expr : vd->lhs) {
        resolve_expr(expr);
    }

    for (Ast *expr : vd->rhs) {
        resolve_expr(expr);
    }
}

void resolve_top_level_stmt(Resolver *R, Ast *stmt) {
    switch (stmt->kind) {
        case Ast_ValueDecl: {
            resolve_value_decl(static_cast<AstValueDecl*>(stmt));
            break;
        }
    }
}


void resolve_file(Resolver *R, AstFile *file) {
    for (Ast *stmt : file->decls) {
        resolve_top_level_stmt(R, stmt);
    }
}
