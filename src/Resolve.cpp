#include "Ast.h"
#include "Resolve.h"

void resolve_expr(Resolver *R, Ast *expr) {
    switch (expr->kind) {
        default:
            break;
    }
}

void resolve_value_decl(Resolver *R, AstValueDecl *vd) {
    for (Ast *expr : vd->lhs) {
        resolve_expr(R, expr);
    }

    for (Ast *expr : vd->rhs) {
        resolve_expr(R, expr);
    }

    if (vd->type_defn) {
        resolve_expr(R, vd->type_defn);

        for (Ast *rhs : vd->rhs) {
            // if (!type_check(vd->type_defn->type, rhs->type)) {
            //     report_error(R, "Types do not match");
            // }
        }
    }

}

void resolve_top_level_stmt(Resolver *R, Ast *stmt) {
    switch (stmt->kind) {
        default:
            break;
        case Ast_ValueDecl:
            resolve_value_decl(R, static_cast<AstValueDecl*>(stmt));
            break;
    }
}


void resolve_file(Resolver *R, AstFile *file) {
    for (Ast *stmt : file->decls) {
        resolve_top_level_stmt(R, stmt);
    }
}
