#include <assert.h>
#include <print>

#include "Atom.h"
#include "Ast.h"
#include "Resolve.h"
#include "Report.h"


//NOTE: Use for hasing atoms
// uint64_t hash_pointer(void* ptr) {
//     uint64_t x = (uintptr_t)ptr;
//     x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
//     x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
//     x = x ^ (x >> 31);
//     return x;
// }

void resolve_name(Resolver *R, AstName *name) {
}

void resolve_expr(Resolver *R, Ast *expr) {
    switch (expr->kind) {
        default:
            break;

        case Ast_Name: {
            AstName *name = static_cast<AstName*>(expr);
            resolve_name(R, name);
            break;
        }

        case Ast_LiteralExpr: {
            LiteralExpr *le = static_cast<LiteralExpr*>(expr);
            break;
        }
    }
}

Symbol *symbol_find(Resolver *R, Atom *name) {
    Scope *scope = R->scope;
    for (Symbol *s : scope->symbol_table) {
        if (s->name == name) {
            return s;
        }
    }
    return nullptr;
}

Symbol *symbol_create(Resolver *R) {
    Symbol *s = new Symbol;
    s->scope = R->scope;
    R->scope->symbol_table.add(s);
    return s;
}

void resolve_value_decl(Resolver *R, AstValueDecl *vd) {
    for (Ast *expr : vd->lhs) {
        assert(expr->kind == Ast_Name);

        AstName *name = static_cast<AstName*>(expr);

        Symbol *lookup = symbol_find(R, name->name);
        if (lookup) {
            report_error(name, "found duplicate of '{}'", get_string(name->name));
        } else {
            Symbol *symbol = symbol_create(R);
            symbol->name = name->name;
            symbol->node = name;
            name->symbol = symbol;
        }
    }

    for (Ast *expr : vd->rhs) {
        resolve_expr(R, expr);
    }

    if (vd->type_defn) {
        resolve_expr(R, vd->type_defn);

        for (Ast *rhs : vd->rhs) {
            // if (!type_check(vd->type_defn->type, rhs->type))
            if (vd->type_defn->type == rhs->type) {
                report_error(rhs, "type of expression does not match");
            }
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
    Scope *global_scope = new Scope;
    R->scope = global_scope;
    for (Ast *stmt : file->decls) {
        resolve_top_level_stmt(R, stmt);
    }
}
