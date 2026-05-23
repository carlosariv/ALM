#include <assert.h>
#include <print>

#include "Atom.h"
#include "Ast.h"
#include "Resolve.h"
#include "Report.h"
#include "Types.h"

template <typename T>
const T& Min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T>
const T& Max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

//NOTE: Use for hasing atoms
// uint64_t hash_pointer(void* ptr) {
//     uint64_t x = (uintptr_t)ptr;
//     x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
//     x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
//     x = x ^ (x >> 31);
//     return x;
// }

ComptimeValue ct_value_int(int int_val) {
    ComptimeValue val = {};
    val.kind = ComptimeValue_Integer;
    val.integer_value = int_val;
    return val;
}

void report_redeclaration(Ident *name) {
    report_error(name, "redeclaration of '{}'", get_string(name->name));
}

Scope *scope_create(Scope *parent, Ast *node) {
    Scope *scope = new Scope;
    scope->parent = parent;
    scope->node = node;
    return scope;
}

Decl *decl_create(Scope *scope, Atom *name) {
    Decl *decl = new Decl();
    decl->name = name;
    decl->scope = scope;
    if (scope) {
        scope->decl_table.add(decl);
    }
    return decl;
}

Decl *decl_find(Scope *scope, Atom *name) {
    for (Decl *s : scope->decl_table) {
        if (s->name == name) {
            return s;
        }
    }
    return nullptr;
}

Decl *decl_lookup(Scope *scope, Atom *name) {
    while (scope != nullptr) {
        for (Decl *s : scope->decl_table) {
            if (s->name == name) {
                return s;
            }
        }
        scope = scope->parent;
    }
    return nullptr;
}

Ast *find_control_target(Scope *scope, AstKind stmt) {
    while (scope) {
        Ast *node = scope->node;
        if (node == nullptr) break;

        bool valid = false;
        switch (stmt) {
            default:
                break;
            case Ast_Fallthrough:
                valid = node->kind == Ast_IfCaseExpr;
                break;
            case Ast_Continue:
                valid = node->kind == Ast_While || node->kind == Ast_For || node->kind == Ast_Do;
                break;
            case Ast_Break:
                valid = node->kind == Ast_IfCaseExpr || node->kind == Ast_While || node->kind == Ast_For || node->kind == Ast_Do;
                break;
            case Ast_Return:
                valid = node->kind == Ast_ProcLit;
                break;
        }

        if (valid) {
            return node;
        }

        scope = scope->parent;
    }
    return nullptr;
}

void add_global_constant_int(Resolver *R, String name, int val) {
    ComptimeValue ct_val = ct_value_int(val);
    Decl *decl = decl_create(R->global_scope, atom_create(name));
    decl->kind = Decl_Constant;
    decl->type = t_i64;
    decl->resolve_state = ResolveState_Completed;
}

void resolve_value_decl(Resolver *R, ValueDecl *vd, bool is_global);
void resolve_expr(Resolver *R, Ast *expr);
void resolve_stmt(Resolver *R, Ast *stmt);

void resolve_name(Resolver *R, Ident *name) {
    Scope *scope = R->scope;
    Decl *decl = decl_lookup(scope, name->name);

    if (decl != nullptr) {
        name->ref = decl;
        name->type = decl->type;
    } else {
        report_error(name, "could not find identifier'{}'", get_string(name->name));
    }
}

void resolve_literal_expr(Resolver *R, LiteralExpr *le) {
    ComptimeValue value = {};
    Type *type = nullptr;
    switch (le->literal_kind) {
        case Literal_Integer:
            value.kind = ComptimeValue_Integer;
            value.integer_value = le->integer_value;
            type = t_u64;
            break;
        case Literal_Floating:
            value.kind = ComptimeValue_Float;
            value.float_value = le->float_value;
            type = t_f64;
            break;
        case Literal_String:
            value.kind = ComptimeValue_String;
            value.string_value = le->string_value;
            type = t_string;
            break;
    }

    le->is_comptime = true;
    le->ct_value = value;
    le->type = type;
}

void resolve_unary_expr(Resolver *R, UnaryExpr *ue) {
    resolve_expr(R, ue->operand);
    ue->type = ue->operand->type;
}

void resolve_binary_expr(Resolver *R, BinaryExpr *be) {
    resolve_expr(R, be->lhs);
    resolve_expr(R, be->rhs);

    if (type_match(be->lhs->type, be->rhs->type)) {
        report_error(be, "types of operands mismatch");
    }
}

void resolve_selector_expr(Resolver *R, SelectorExpr *se) {
    resolve_expr(R, se->operand);
}

void resolve_subscript_expr(Resolver *R, SubscriptExpr *se) {
    resolve_expr(R, se->operand);

    if (se->value) {
        resolve_expr(R, se->value);
    }
}

int get_type_arity(Array<Ast*> list) {
    int count = 0;
    for (Ast *elem : list) {
        count += get_type_arity(elem->type);
    }
    return count;
}

bool check_argument_procedure_type_match(Array<Ast*> arguments, TupleType *params) {
    if (get_type_arity(arguments) != get_type_arity(params)) {
        return false;
    }

    for (int arg_idx = 0, param_idx = 0; arg_idx < arguments.count; arg_idx++) {
        Ast *arg = arguments[arg_idx];
        if (arg->type->kind == Type_Tuple) {
            TupleType *tup = static_cast<TupleType*>(arg->type);
            int arg_arity = get_type_arity(tup);
            for (int t = 0; t < arg_arity; t++) {
                Type *param_type = params->types[param_idx];
                Type *arg_type = tup->types[t];
                if (!type_match(param_type, arg_type)) {
                    return false;
                }
                param_idx++;
            }
        } else {
            Type *param_type = params->types[param_idx];
            if (!type_match(param_type, arg->type)) {
                return false;
                break;
            }
            param_idx++;
        }
    }
    return true;
}


void resolve_call_expr(Resolver *R, CallExpr *ce) {
    resolve_expr(R, ce->operand);

    for (Ast *arg : ce->arguments) {
        resolve_expr(R, arg);
    }

    if (ce->operand->kind == Ast_Ident) {
        Ident *name = static_cast<Ident*>(ce->operand);
        Decl *proc_group = name->ref;

        if (proc_group) {
            ProcLit *callee = nullptr;

            for (Decl *proc : proc_group->procedures) {
                ProcLit *proc_lit = proc->proc_lit;
                ProcType *proc_type = static_cast<ProcType*>(proc_lit->proc_type->type);

                if (check_argument_procedure_type_match(ce->arguments, proc_type->params)) {
                    callee = proc_lit;
                    break;
                }
            }

            if (callee) {
                ProcType *proc_type = static_cast<ProcType*>(callee->proc_type->type);
                ce->type = proc_type->results;
            } else {
                if (proc_group->procedures.count == 1) {
                    report_error(ce, "invalid argument types for procedure '{}'", get_string(name->name));
                } else {
                    report_error(ce, "no procedure with matching argument types found");
                }
            }
        }
    } else {
        if (ce->operand->type->kind == Type_Proc) {
            ProcType *proc_type = static_cast<ProcType*>(ce->operand->type);
            if (check_argument_procedure_type_match(ce->arguments, proc_type->params)) {
                ce->type = proc_type->results;
            } else {
                report_error(ce, "invalid argument types for callee");
            }
        } else {
            report_error(ce->operand, "operand is not a procedure type");
        }
    }
}

void resolve_paren_expr(Resolver *R, ParenExpr *pe) {
    resolve_expr(R, pe->expr);
}

void resolve_block_expr(Resolver *R, BlockExpr *block) {
    for (Ast *stmt : block->statements) {
        resolve_stmt(R, stmt);
    }
}

void resolve_compound_literal(Resolver *R, CompoundLiteralExpr *comp) {
    resolve_expr(R, comp->operand);

    for (Ast *v : comp->initializer_list) {
        resolve_expr(R, v);
    }
}


void resolve_if_expr(Resolver *R, IfExpr *if_expr) {
    resolve_expr(R, if_expr->condition);

    resolve_expr(R, if_expr->then_expr);
}

void resolve_ifcase_expr(Resolver *R, IfCaseExpr *ifcase_expr) {
    resolve_expr(R, ifcase_expr->condition);

    resolve_expr(R, ifcase_expr->block);
}

void resolve_star_expr(Resolver *R, StarExpr *star_expr) {
    resolve_expr(R, star_expr->elem);
}

void resolve_deref_expr(Resolver *R, DerefExpr *deref_expr) {
    resolve_expr(R, deref_expr->operand);
}

void resolve_proc_type(Resolver *R, ProcTypeDefn *proc_type) {
    for (ValueDecl *param : proc_type->params) {
        resolve_value_decl(R, param, false);
    }

    for (Ast *ret : proc_type->results) {
        resolve_expr(R, ret);
    }
}

void resolve_proc_lit(Resolver *R, ProcLit *proc_lit) {
    Scope *scope = scope_create(R->scope, proc_lit);
    R->scope = scope;

    resolve_proc_type(R, proc_lit->proc_type);
    if (proc_lit->body) {
        resolve_block_expr(R, proc_lit->body);
    }

    R->scope = scope->parent;
}

void resolve_array_type(Resolver *R, ArrayTypeDefn *array_type) {
    resolve_expr(R, array_type->operand);

    if (array_type->size) {
        resolve_expr(R, array_type->size);
    }
}

void resolve_struct_type(Resolver *R, StructTypeDefn *type_defn) {
    StructType *st = ast_new<StructType>();
    type_defn->type = st;

    Scope *scope = scope_create(R->scope, type_defn);
    R->scope = scope;

    for (ValueDecl *member : type_defn->members) {
        resolve_value_decl(R, member, false);

        if (member->is_mutable) {
            st->members.append(member->type);
        }
    }

    R->scope = scope->parent;
}

void resolve_union_type(Resolver *R, UnionTypeDefn *union_type) {
}

void resolve_enum_type(Resolver *R, EnumTypeDefn *enum_type) {
}

void resolve_enumerator(Resolver *R, Enumerator *enumerator) {
}

void resolve_expr(Resolver *R, Ast *expr) {
    switch (expr->kind) {
        default:
            assert(0);
            break;

        case Ast_Ident:
            resolve_name(R, (Ident *)expr);
            break;
        case Ast_LiteralExpr:
            resolve_literal_expr(R, (LiteralExpr *)expr);
            break;
        case Ast_UnaryExpr:
            resolve_unary_expr(R, (UnaryExpr *)expr);
            break;
        case Ast_BinaryExpr:
            resolve_binary_expr(R, (BinaryExpr *)expr);
            break;
        case Ast_SelectorExpr:
            resolve_selector_expr(R, (SelectorExpr *)expr);
            break;
        case Ast_SubscriptExpr:
            resolve_subscript_expr(R, (SubscriptExpr *)expr);
            break;
        case Ast_CallExpr:
            resolve_call_expr(R, (CallExpr *)expr);
            break;
        case Ast_ParenExpr:
            resolve_paren_expr(R, (ParenExpr *)expr);
            break;
        case Ast_BlockExpr:
            resolve_block_expr(R, (BlockExpr *)expr);
            break;
        case Ast_CompoundLiteral:
            resolve_compound_literal(R, (CompoundLiteralExpr *)expr);
            break;
        case Ast_IfExpr:
            resolve_if_expr(R, (IfExpr *)expr);
            break;
        case Ast_IfCaseExpr:
            resolve_ifcase_expr(R, (IfCaseExpr *)expr);
            break;
        case Ast_StarExpr:
            resolve_star_expr(R, (StarExpr *)expr);
            break;
        case Ast_DerefExpr:
            resolve_deref_expr(R, (DerefExpr *)expr);
            break;

        case Ast_ProcType:
            resolve_proc_type(R, (ProcTypeDefn *)expr);
            break;
        case Ast_ProcLit:
            resolve_proc_lit(R, (ProcLit *)expr);
            break;

        case Ast_ArrayType:
            resolve_array_type(R, (ArrayTypeDefn *)expr);
            break;
        case Ast_StructType:
            resolve_struct_type(R, (StructTypeDefn *)expr);
            break;
        case Ast_UnionType:
            resolve_union_type(R, (UnionTypeDefn *)expr);
            break;
        case Ast_EnumType:
            resolve_enum_type(R, (EnumTypeDefn *)expr);
            break;
        case Ast_Enumerator:
            resolve_enumerator(R, (Enumerator *)expr);
            break;
    }
}

void resolve_value_decl(Resolver *R, ValueDecl *vd, bool is_global) {
    Scope *scope = R->scope;

    if (!vd->is_mutable) {
        if (vd->lhs.count != vd->rhs.count) {
            report_error(vd, "number of right hand values does not match number of left side, in constant value declaration");
        }
    }

    if (is_global) {
        if (vd->is_mutable) {
            if (vd->lhs.count <= vd->rhs.count) {
            } else {
                report_error(vd, "too many values on right hand side");
            }
        }
    } else {
        for (Ast *expr : vd->lhs) {
            assert(expr->kind == Ast_Ident);
            Ident *name = static_cast<Ident*>(expr);

            Decl *lookup = decl_find(scope, name->name);
            if (lookup) {
                report_redeclaration(name);
            } else {
                Decl *Decl = decl_create(scope, name->name);
                Decl->vd = vd;
                name->ref = Decl;
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

Decl *proc_decl_create(Atom *name, ProcLit *proc_lit, ValueDecl *vd) {
    Decl *proc = decl_create(nullptr, name);
    proc->kind = Decl_Proc;
    proc->proc_lit = proc_lit;
    proc->vd = vd;
    return proc;
}

void register_value_decl(Resolver *R, Scope *scope, ValueDecl *vd) {
    if (!vd->is_mutable) {
        assert(vd->lhs.count == vd->rhs.count);
        int value_count = Min(vd->lhs.count, vd->rhs.count);
        for (int i = 0 ; i < value_count; i++) {
            Ident *name = static_cast<Ident*>(vd->lhs[i]);
            Ast *rhs = vd->rhs[i];

            Decl *lookup = decl_find(scope, name->name);
            if (lookup) {
                name->ref = lookup;

                if (lookup->kind == Decl_ProcGroup) {
                    if (rhs->kind == Ast_ProcLit) {
                        Decl *proc = proc_decl_create(name->name, static_cast<ProcLit*>(rhs), vd);
                        name->ref = proc;
                        lookup->procedures.append(proc);
                    } else {
                        report_redeclaration(name);
                    }
                } else if (rhs->kind == Ast_ProcLit) {
                    report_redeclaration(name);
                }
            } else {
                Decl *decl = decl_create(scope, name->name);
                //NOTE: Register procedure groups to not falsely report redeclaration
                if (rhs->kind == Ast_ProcLit) {
                    decl->kind = Decl_ProcGroup;
                    Decl *proc = proc_decl_create(name->name, static_cast<ProcLit*>(rhs), vd);
                    name->ref = proc;
                    decl->procedures.append(proc);
                } else {
                    decl->vd = vd;
                    name->ref = decl;
                }
            }
        }
    } else {
        for (int i = 0; i < vd->lhs.count; i++) {
            Ident *name = static_cast<Ident*>(vd->lhs[i]);
            Decl *lookup = decl_find(scope, name->name);
            if (!lookup) {
                Decl *decl = decl_create(scope, name->name);
                decl->kind = Decl_Var;
                decl->vd = vd;
                name->ref = decl;
            } else {
                report_redeclaration(name);
            }
        }
    }
}

void register_top_level_stmt(Resolver *R, Ast *node) {
    switch (node->kind) {
        default:
            break;

        case Ast_ValueDecl:
            register_value_decl(R, R->global_scope, static_cast<ValueDecl*>(node));
            break;
    }
}

void resolve_assign_stmt(Resolver *R, AssignStmt *assign) {
    for (Ast *lhs : assign->lhs) {
        resolve_expr(R, lhs);
    }

    for (Ast *rhs : assign->rhs) {
        resolve_expr(R, rhs);
    }

    //TODO: Type matching
}

void resolve_break_stmt(Resolver *R, BreakStmt *break_stmt) {
    if (break_stmt->expr) {
        resolve_expr(R, break_stmt->expr);
    }

    Scope *scope = R->scope;
    Ast *control = find_control_target(scope, Ast_Break);

    if (control == nullptr) {
        report_error(break_stmt, "illegal break outside of a loop");
    }
}

void resolve_continue_stmt(Resolver *R, ContinueStmt *cont_stmt) {
    Scope *scope = R->scope;
    Ast *control = find_control_target(scope, Ast_Continue);

    if (control == nullptr) {
        report_error(cont_stmt, "illegal continue outside of a loop");
    }
}

void resolve_fallthrough_stmt(Resolver *R, FallthroughStmt *fallthrough_stmt) {
    Scope *scope = R->scope;
    Ast *control = find_control_target(scope, Ast_Fallthrough);

    if (control == nullptr) {
        report_error(fallthrough_stmt, "illegal fallthrough outside of an ifcase");
    }
}

void resolve_return_stmt(Resolver *R, ReturnStmt *return_stmt) {
    Scope *scope = R->scope;
    Ast *control = find_control_target(scope, Ast_Return);

    if (control == nullptr) {
        report_error(return_stmt, "illegal return outside of a procedure");
    }

    for (Ast *result : return_stmt->results) {
        resolve_expr(R, result);
    }
}

void resolve_while_stmt(Resolver *R, WhileStmt *while_stmt) {
    Scope *scope = scope_create(R->scope, while_stmt);
    R->scope = scope;

    resolve_expr(R, while_stmt->condition);
    resolve_expr(R, while_stmt->block);

    R->scope = scope->parent;
}

void resolve_do_stmt(Resolver *R, DoStmt *do_stmt) {
    Scope *scope = scope_create(R->scope, do_stmt);
    R->scope = scope;

    resolve_expr(R, do_stmt->condition);
    resolve_expr(R, do_stmt->block);

    R->scope = scope->parent;
}

void resolve_for_stmt(Resolver *R, ForStmt *for_stmt) {
    Scope *scope = scope_create(R->scope, for_stmt);
    R->scope = scope;

    resolve_expr(R, for_stmt->condition);
    resolve_expr(R, for_stmt->block);

    R->scope = scope->parent;
}

void resolve_case_expr(Resolver *R, CaseExpr *case_expr) {
    if (case_expr->expr) {
        resolve_expr(R, case_expr->expr);
    }

    Scope *scope = scope_create(R->scope, case_expr);
    R->scope = scope;

    for (Ast *stmt : case_expr->statements) {
        resolve_stmt(R, stmt);
    }

    R->scope = scope->parent;
}

void resolve_stmt(Resolver *R, Ast *stmt) {
    bool is_global = R->scope == R->global_scope;

    switch (stmt->kind) {
        default:
            assert(0);
            break;

        case Ast_IfExpr:
            resolve_if_expr(R, (IfExpr *)stmt);
            break;
        case Ast_IfCaseExpr:
            resolve_ifcase_expr(R, (IfCaseExpr *)stmt);
            break;

        case Ast_ValueDecl:
            resolve_value_decl(R, (ValueDecl *)stmt, is_global);
            break;
        case Ast_Assign:
            resolve_assign_stmt(R, (AssignStmt *)stmt);
            break;
        case Ast_ExprStmt:
            resolve_expr(R, ((ExprStmt *)stmt)->expr);
            break;
        case Ast_EmptyStmt:
            break;
        case Ast_Break:
            resolve_break_stmt(R, (BreakStmt *)stmt);
            break;
        case Ast_Continue:
            resolve_continue_stmt(R, (ContinueStmt *)stmt);
            break;
        case Ast_Fallthrough:
            resolve_fallthrough_stmt(R, (FallthroughStmt *)stmt);
            break;
        case Ast_Return:
            resolve_return_stmt(R, (ReturnStmt *)stmt);
            break;
        case Ast_Case:
            resolve_case_expr(R, (CaseExpr *)stmt);
            break;
        case Ast_Do:
            resolve_do_stmt(R, (DoStmt *)stmt);
            break;
        case Ast_While:
            resolve_while_stmt(R, (WhileStmt *)stmt);
            break;
        case Ast_For:
            resolve_for_stmt(R, (ForStmt *)stmt);
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
        Decl *Decl = decl_create(scope, name);
        Decl->kind = Decl_Type;
        Decl->resolve_state = ResolveState_Completed;
        Decl->type = type;
    }
}

void resolve_program(Resolver *R, Parser *P) {
    Scope *global_scope = scope_create(nullptr, nullptr);
    R->global_scope = global_scope;
    R->scope = global_scope;

    register_builtin_types(R);

    add_global_constant_int(R, STRZ("true"), 1);
    add_global_constant_int(R, STRZ("false"), 0);

    for (AstFile *file : R->files) {
        for (Ast *stmt : file->decls) {
            register_top_level_stmt(R, stmt);
        }
    }

    //NOTE: Compute actual type of string type
    Decl *string_type = decl_find(global_scope, atom_create(STRZ("string")));
    resolve_top_level_stmt(R, string_type->vd);
    t_string = string_type->type;

    for (AstFile *file : R->files) {
        for (Ast *stmt : file->decls) {
            resolve_top_level_stmt(R, stmt);
        }
    }

}
