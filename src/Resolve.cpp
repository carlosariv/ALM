#include <assert.h>
#include <print>

#include "Atom.h"
#include "Ast.h"
#include "Resolve.h"
#include "Report.h"
#include "Types.h"


//NOTE: Use for hasing atoms
// uint64_t hash_pointer(void* ptr) {
//     uint64_t x = (uintptr_t)ptr;
//     x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
//     x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
//     x = x ^ (x >> 31);
//     return x;
// }
//

Symbol *symbol_create(Scope *scope, Atom *name) {
    Symbol *s = new Symbol;
    s->name = name;
    s->scope = scope;
    s->state = SymbolState_Unresolved;
    scope->symbol_table.add(s);
    return s;
}

Symbol *symbol_find(Scope *scope, Atom *name) {
    for (Symbol *s : scope->symbol_table) {
        if (s->name == name) {
            return s;
        }
    }
    return nullptr;
}

Symbol *symbol_lookup(Scope *scope, Atom *name) {
    while (scope != nullptr) {
        for (Symbol *s : scope->symbol_table) {
            if (s->name == name) {
                return s;
            }
        }
        scope = scope->parent;
    }
    return nullptr;
}

void resolve_name(Resolver *R, Ident *name) {
    Scope *scope = R->scope;
    Symbol *sym = symbol_lookup(scope, name->name);

    if (sym != nullptr) {
        name->symbol = sym;
        name->type = sym->type;
    } else {
        report_error(name, "could not find identifier'{}'", get_string(name->name));
    }
}

void resolve_literal_expr(Resolver *R, LiteralExpr *le) {
    switch (le->literal_kind) {

    }
}

void resolve_expr(Resolver *R, Ast *expr) {
    switch (expr->kind) {
        default:
            break;

        case Ast_Ident: {
            Ident *name = static_cast<Ident*>(expr);
            resolve_name(R, name);
            break;
        }

        case Ast_LiteralExpr:
            resolve_literal_expr((LiteralExpr *)expr);
            break;

        case Ast_UnaryExpr: {
            break;
        }
        case Ast_BinaryExpr: {
            break;
        }
        case Ast_SelectorExpr: {
            break;
        }
        case Ast_SubscriptExpr: {
            break;
        }
        case Ast_CallExpr: {
            break;
        }
        case Ast_ParenExpr: {
            break;
        }
        case Ast_BlockExpr: {
            break;
        }
        case Ast_CompoundLiteral: {
            break;
        }
        case Ast_IfExpr: {
            break;
        }
        case Ast_IfCaseExpr: {
            break;
        }
        case Ast_StarExpr: {
            break;
        }
        case Ast_DerefExpr: {
            break;
        }

        case Ast_ProcType: {
            break;
        }
        case Ast_ProcLit: {
            break;
        }
        case Ast_Param: {
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
        case Ast_EnumType: {
            break;
        }
        case Ast_Enumerator: {
            break;
        }
    }
}

bool type_match(Type *lhs, Type *rhs) {
    if (lhs == rhs) return true;
    return false;
}

void resolve_value_decl(Resolver *R, ValueDecl *vd, bool is_global) {
    Scope *scope = R->scope;

    if (!is_global) {
        for (Ast *expr : vd->lhs) {
            assert(expr->kind == Ast_Ident);

            Ident *name = static_cast<Ident*>(expr);

            Symbol *lookup = symbol_find(scope, name->name);
            if (lookup) {
                report_error(name, "redefinition of '{}'", get_string(name->name));
            } else {
                Symbol *symbol = symbol_create(scope, name->name);
                symbol->vd = vd;
                name->symbol = symbol;
            }
        }
    }

    if (vd->type_defn) {
        resolve_expr(R, vd->type_defn);
    }

    for (Ast *expr : vd->rhs) {
        resolve_expr(R, expr);
    }

    if (vd->type_defn) {
        for (Ast *rhs : vd->rhs) {
            if (!type_match(vd->type_defn->type, rhs->type)) {
                report_error(rhs, "right hand side of declaration does not match specified type");
            }
        }
    }
}

void register_global_value_decl(Resolver *R, ValueDecl *vd) {
    Scope *scope = R->global_scope;
    for (Ast *lhs : vd->lhs) {
        Ident *name = static_cast<Ident*>(lhs);
        Symbol *sym = symbol_create(scope, name->name);
        sym->kind = Symbol_Var;
        sym->vd = vd;
        name->symbol = sym;
    }
}

void register_top_level_stmt(Resolver *R, Ast *node) {
    switch (node->kind) {
        default:
            break;

        case Ast_ValueDecl:
            register_global_value_decl(R, static_cast<ValueDecl*>(node));
            break;
    }
}

void resolve_top_level_stmt(Resolver *R, Ast *node) {
    switch (node->kind) {
        default:
            break;

        case Ast_ValueDecl: {
            ValueDecl *vd = static_cast<ValueDecl*>(node);
            resolve_value_decl(R, vd, true);
            break;
        }
    }
}

void register_builtin_types(Resolver *R) {
    Scope *scope = R->global_scope;
    for (TypeKind kind = Type_BuiltinBegin; kind < Type_BuiltinEnd; kind = (TypeKind)(kind + 1)) {
        Type *type = &g_builtin_types[kind];
        Atom *name = atom_create(type->name);
        Symbol *symbol = symbol_create(scope, name);
        symbol->kind = Symbol_Type;
        symbol->state = SymbolState_Resolved;
        symbol->type = type;
    }
}


void resolve_file(Resolver *R, AstFile *file) {
    Scope *global_scope = new Scope;
    R->scope = global_scope;

    register_builtin_types(R);

    for (Ast *stmt : file->decls) {
        register_top_level_stmt(R, stmt);
    }

    for (Ast *stmt : file->decls) {
        resolve_top_level_stmt(R, stmt);
    }
}
