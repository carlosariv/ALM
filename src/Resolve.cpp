// Value Declarations:
// There are declarations that are out of order such as globals and declarations within type definitions.
// In order declarations are evaluated as they appear.
// Out of order declarations are first forward declared so that they can be evaluated later when referenced.

#include <assert.h>
#include <print>
#include <algorithm>

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

void resolve_value_decl_stmt(Resolver *R, ValueDecl *vd);

bool is_proc_lit(Ast *node) {
    return node->kind == Ast_ProcLit;
}

//NOTE: To determine if an immutable value declaration is a type declaration or a constant value declaration
bool is_ast_type(Ast *node) {
    switch (node->kind) {
        case Ast_StarExpr:
        case Ast_ProcType:
        case Ast_ProcLit:
        case Ast_ArrayType:
        case Ast_StructType:
        case Ast_UnionType:
        case Ast_EnumType:
            return true;
        default:
            return false;
    }
}

ComptimeValue ct_value_int(int int_val) {
    ComptimeValue val = {};
    val.kind = ComptimeValue_Integer;
    val.integer_value = int_val;
    return val;
}

void report_redeclaration(Ident *name) {
    report_error(name, "redeclaration of '{}'", name->name);
}

Scope *scope_create(Scope *parent, Ast *node) {
    Scope *scope = new Scope;
    scope->parent = parent;
    scope->node = node;
    return scope;
}

void scope_insert(Scope *scope, Decl *decl) {
    decl->scope = scope;
    scope->decl_table.append(decl);
}

Decl *decl_new(DeclKind kind, Atom *name) {
    Decl *decl = new Decl;
    decl->kind = kind;
    decl->name = name;
    return decl;
}

Decl *decl_var_create(Scope *scope, Atom *name) {
    Decl *decl = decl_new(Decl_Var, name);
    if (scope) {
        scope_insert(scope, decl);
    }
    return decl;
}

Decl *decl_proc_group_create(Scope *scope, Atom *name) {
    Decl *decl = decl_new(Decl_ProcGroup, name);
    scope_insert(scope, decl);
    return decl;
}

Decl *decl_proc_create(Decl *proc_group, Atom *name, ProcLit *proc_lit) {
    Decl *decl = decl_new(Decl_Proc, name);
    decl->proc_lit = proc_lit;
    proc_group->procedures.append(decl);
    return decl;
}

Decl *decl_type_create(Scope *scope, Atom *name, Ast *value) {
    Decl *decl = decl_new(Decl_Type, name);
    decl->type_defn = value;
    scope_insert(scope, decl);
    return decl;
}

Decl *decl_constant_create(Scope *scope, Atom *name, Ast *value) {
    Decl *decl = decl_new(Decl_Constant, name);
    decl->init_expr = value;
    scope_insert(scope, decl);
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
                valid = node->kind == Ast_For;
                break;
            case Ast_Break:
                valid = node->kind == Ast_IfCaseExpr || node->kind == Ast_For;
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
    Decl *decl = decl_constant_create(R->global_scope, atom_create(name), nullptr);
    decl->type = t_i64;
    decl->resolve_state = ResolveState_Completed;
}

void resolve_value_decl(Resolver *R, ValueDecl *vd, bool is_global);
void resolve_expr(Resolver *R, Ast *expr);
void resolve_expr_base(Resolver *R, Ast *expr);
void resolve_stmt(Resolver *R, Ast *stmt);
void resolve_decl(Resolver *R, Decl *decl);
void resolve_type(Resolver *R, Ast *expr);

void resolve_param(Resolver *R, Param *param);
void resolve_proc_type(Resolver *R, ProcTypeDefn *pt);
void resolve_proc_signature(Resolver *R, ProcLit *proc_lit);
void resolve_proc_lit(Resolver *R, ProcLit *proc_lit);

void resolve_name(Resolver *R, Ident *name) {
    Scope *scope = R->scope;
    Decl *decl = decl_lookup(scope, name->name);
    name->ref = decl;

    if (decl) {
        if (decl->kind == Decl_ProcGroup) {
            for (int i = 0; i < decl->procedures.count; i++) {
                Decl *proc = decl->procedures[i];
                resolve_proc_signature(R, proc->proc_lit);
                proc->type = proc->proc_lit->proc_type->type;
            }
        } else {
            //@Todo: Should this be called on a local value declaration statement?
            // Local value decls are not filled out with the init expressions because of multiple-type values
            resolve_decl(R, decl);
            name->type = decl->type;

            switch (decl->kind) {
                case Decl_Nil:
                    break;
                case Decl_Var:
                    name->mode = AddressingMode_Variable;
                    break;
                case Decl_Type:
                    name->mode = AddressingMode_Type;
                    break;
                case Decl_ProcGroup:
                case Decl_Proc:
                    name->mode = AddressingMode_Procedure;
                    break;
                case Decl_Constant:
                    if (decl->flags & DeclFlag_ConstantType) {
                        name->mode = AddressingMode_Type;
                    } else if (decl->flags & DeclFlag_ConstantProcedure) {
                        name->mode = AddressingMode_Procedure;
                    } else {
                        name->mode = AddressingMode_Constant;
                    }
                    break;
            }
        }
    } else {
        report_error(name, "could not find identifier '{}'", name->name);
        name->type = t_invalid;
    }
}

void resolve_literal_expr(Resolver *R, LiteralExpr *le) {
    ComptimeValue value = {};
    Type *type = nullptr;
    switch (le->literal_kind) {
        case Literal_Integer:
            value.kind = ComptimeValue_Integer;
            value.integer_value = le->integer_value;
            if (value.integer_value <= 255) {
                type = t_u8;
            } else if (value.integer_value <= 65535) {
                type = t_u16;
            } else if (value.integer_value <= 16777215) {
                type = t_u32;
            } else {
                type = t_u64;
            }
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

    le->mode = AddressingMode_Constant;
    le->ct_value = value;
    le->type = type;
}

void resolve_unary_expr(Resolver *R, UnaryExpr *ue) {
    resolve_expr(R, ue->operand);

    Ast *operand = ue->operand;

    switch (ue->op) {
        default:
            break;
        case Operator_UnaryPlus:
            if (is_arithmetic_type(operand->type)) {
                ue->type = operand->type;
                ue->mode = AddressingMode_Value;
            } else {
                report_error(ue, "bad operator '+' on a non-arithmetic type");
                ue->type = t_invalid;
            }
            break;
        case Operator_Negate:
            if (is_arithmetic_type(operand->type)) {
                ue->type = get_signed_type(operand->type);
                ue->mode = AddressingMode_Value;
            } else {
                report_error(ue, "bad operator '-' on a non-arithmetic type");
                ue->type = t_invalid;
            }
            break;
        case Operator_AddressOf:
            if (operand->mode == AddressingMode_Variable) {
                ue->type = pointer_type_create(ue->type);
                ue->mode = AddressingMode_Variable;
            } else {
                report_error(ue, "cannot take address of a non r-value expression");
                ue->type = t_invalid;
            }
            break;
    }
}

void resolve_binary_expr(Resolver *R, BinaryExpr *be) {
    resolve_expr(R, be->lhs);
    resolve_expr(R, be->rhs);

    //@Todo: Check valid types per binary operator
    if (!types_assignable(be->lhs->type, be->rhs->type)) {
        String lhs_type_string = string_from_type(be->lhs->type);
        String rhs_type_string = string_from_type(be->rhs->type);
        report_error(be, "types of operands mismatch, '{}', '{}'", lhs_type_string, rhs_type_string);
    }

    be->mode = AddressingMode_Value;
    be->type = be->lhs->type;
}

Decl *decl_field_create(Scope *scope, Atom *name, Type *type) {
    Decl *field = decl_var_create(scope, name);
    field->type = type;
    field->resolve_state = ResolveState_Completed;
    return field;
}

Decl *lookup_field(Ast *selector, Atom *name) {
    switch (selector->type->kind) {
        case Type_String: {
            Atom *text_n = atom_create(STRZ("text"));
            Atom *len_n = atom_create(STRZ("len"));
            static Decl *field_text = decl_field_create(nullptr, text_n, pointer_type_create(t_u8));
            static Decl *field_len  = decl_field_create(nullptr, len_n, t_i64);

            if (name == text_n)  {
                return field_text;
            } else if (name == len_n) {
                return field_len;
            }
            break;
        }
        case Type_Struct: {
            StructType *st = static_cast<StructType*>(selector->type);
            return decl_find(st->scope, name);
        }
        case Type_Union: {
            UnionType *ut = static_cast<UnionType*>(selector->type);
            return decl_find(ut->scope, name);
        }
        case Type_Enum: {
            EnumType *et = static_cast<EnumType*>(selector->type);
            return decl_find(et->scope, name);
        }

        default: break;
    }

    return nullptr;
}

void resolve_selector_expr(Resolver *R, SelectorExpr *se) {
    Ast *operand = se->operand;
    resolve_expr_base(R, operand);

    Atom *name = se->name->name;
    Decl *field = lookup_field(operand, name);

    if (field) {
        se->name->ref = field;
        se->type = field->type;
        se->mode = operand->mode;
    } else {
        report_error(se->name, "field '{}' not found", name);
        se->mode = AddressingMode_Invalid;
        se->type = t_invalid;
    }
}

void resolve_proc_signature(Resolver *R, ProcLit *proc_lit) {
    if (proc_lit->check_state >= ProcCheckState_Signature) return;

    resolve_proc_type(R, proc_lit->proc_type);

    proc_lit->check_state = ProcCheckState_Signature;
}

void forward_declare_value_decl(Resolver *R, Scope *scope, ValueDecl *vd, bool is_global) {
    if (vd->is_mutable) {
        for (int i = 0; i < vd->names.count; i++) {
            Ident *name = vd->names[i];
            Decl *lookup = decl_find(scope, name->name);
            if (!lookup) {
                Decl *decl = decl_var_create(scope, name->name);
                decl->node = vd;
                name->ref = decl;
            } else {
                report_redeclaration(name);
            }
        }

        if (is_global) {
            int value_count = vd->values.count;
            if (vd->names.count < vd->values.count) {
                value_count = vd->names.count;
            }

            for (int i = 0; i < vd->names.count; i++) {
                Decl *decl = vd->names[i]->ref;
                decl->type_defn = vd->type_defn;
            }

            for (int i = 0; i < value_count; i++) {
                Decl *decl = vd->names[i]->ref;
                decl->init_expr = vd->values[i];
            }
        }
    } else {
        assert(vd->names.count == vd->values.count);
        int value_count = std::min(vd->names.count, vd->values.count);

        for (int i = 0 ; i < value_count; i++) {
            Ident *name = vd->names[i];
            Ast *value = vd->values[i];

            Decl *lookup = decl_find(scope, name->name);
            if (lookup) {
                //NOTE: Report redeclaration if either of one of current or lookup are not procedures and the other is.
                if (lookup->kind == Decl_ProcGroup) {
                    Decl *proc_group = lookup;
                    if (is_proc_lit(value)) {
                        Decl *proc = decl_proc_create(proc_group, name->name, static_cast<ProcLit*>(value));
                        name->ref = proc;
                        proc->node = vd;
                        proc->proc_lit = static_cast<ProcLit*>(value);
                    } else {
                        report_redeclaration(name);
                    }
                } else if (is_proc_lit(value)) {
                    report_redeclaration(name);
                }
            } else {
                Decl *decl = nullptr;
                if (is_proc_lit(value)) {
                    Decl *proc_group = decl_proc_group_create(scope, name->name);
                    decl = decl_proc_create(proc_group, name->name, static_cast<ProcLit*>(value));
                } else if (is_ast_type(value)) {
                    decl = decl_type_create(scope, name->name, value);
                } else {
                    decl = decl_constant_create(scope, name->name, value);
                }
                name->ref = decl;
                decl->node = vd;
            }
        }
    }

    vd->forward_declared = true;
}

void resolve_type_decl(Resolver *R, Decl *type_decl) {
    assert(type_decl->resolve_state != ResolveState_Completed);
    resolve_type(R, type_decl->type_defn);
    //@Todo: check if expression evals to a type
    type_decl->type = type_decl->type_defn->type;
    type_decl->type->name = to_string(type_decl->name);
}

void resolve_proc_decl(Resolver *R, Decl *proc_decl) {
    resolve_proc_signature(R, proc_decl->proc_lit);

    resolve_proc_lit(R, proc_decl->proc_lit);
}

void resolve_var_decl(Resolver *R, Decl *var_decl) {
    ValueDecl *vd = static_cast<ValueDecl*>(var_decl->node);
    if (vd->type_defn) {
        resolve_type(R, vd->type_defn);
        var_decl->type = vd->type_defn->type;
    }

    if (var_decl->init_expr) {
        resolve_expr(R, var_decl->init_expr);
    }
}

void resolve_constant_decl(Resolver *R, Decl *decl) {
    Ast *init = decl->init_expr;
    resolve_expr_base(R, init);

    switch (init->mode) {
        default:
            break;
        case AddressingMode_Type:
            decl->flags |= DeclFlag_ConstantType;
            break;
        case AddressingMode_Variable:
        case AddressingMode_Value:
            report_error(init, "expected a constant");
            break;
        case AddressingMode_Procedure:
            decl->flags |= DeclFlag_ConstantProcedure;
            break;
    }

    decl->type = init->type;
}

void resolve_decl(Resolver *R, Decl *decl) {
    if (decl->resolve_state == ResolveState_Completed) return;

    if (decl->resolve_state == ResolveState_InProgress) {
        report_error(decl->node, "cyclical resolution of declaration");
        return;
    }

    decl->resolve_state = ResolveState_InProgress;

    switch (decl->kind) {
        case Decl_Nil:
        case Decl_ProcGroup:
            assert(0);
            break;

        case Decl_Type:
            resolve_type_decl(R, decl);
            break;
        case Decl_Var:
            resolve_var_decl(R, decl);
            break;
        case Decl_Constant:
            resolve_constant_decl(R, decl);
            break;
        case Decl_Proc:
            resolve_proc_decl(R, decl);
            break;
    }

    decl->resolve_state = ResolveState_Completed;
}

int type_arity(Array<Ast*> list) {
    int count = 0;
    for (Ast *elem : list) {
        count += type_arity(elem->type);
    }
    return count;
}

bool check_argument_procedure_types_assignable(Array<Ast*> arguments, TupleType *params) {
    if (type_arity(arguments) != type_arity(params)) {
        return false;
    }

    for (int arg_idx = 0, param_idx = 0; arg_idx < arguments.count; arg_idx++) {
        Ast *arg = arguments[arg_idx];
        if (arg->type->kind == Type_Tuple) {
            TupleType *tup = static_cast<TupleType*>(arg->type);
            int arg_arity = type_arity(tup);
            for (int t = 0; t < arg_arity; t++) {
                Type *param_type = params->types[param_idx];
                Type *arg_type = tup->types[t];
                if (!types_assignable(param_type, arg_type)) {
                    return false;
                }
                param_idx++;
            }
        } else {
            Type *param_type = params->types[param_idx];
            if (!types_assignable(param_type, arg->type)) {
                return false;
                break;
            }
            param_idx++;
        }
    }
    return true;
}

ProcLit *find_procedure_overloaded(Decl *proc_group, Array<Ast*> arguments) {
    for (Decl *proc : proc_group->procedures) {
        ProcLit *proc_lit = proc->proc_lit;
        ProcType *proc_type = static_cast<ProcType*>(proc_lit->proc_type->type);

        if (check_argument_procedure_types_assignable(arguments, proc_type->params)) {
            return proc_lit;
        }
    }
    return nullptr;
}

TupleType *tuple_from_values(Array<Ast*> values) {
    TupleType *tuple = type_new<TupleType>();
    for (Ast *val : values) {
        assert(val->type != nullptr);
        tuple->types.append(val->type);
    }
    return tuple;
}

void resolve_call_expr(Resolver *R, CallExpr *ce) {
    for (Ast *arg : ce->arguments) {
        resolve_expr(R, arg);
    }

    Ast *operand = ce->operand;
    resolve_expr(R, operand);

    bool is_valid = false;

    ProcType *proc_type = nullptr;
    if (operand->kind == Ast_Ident && static_cast<Ident*>(operand)->ref->kind == Decl_ProcGroup) {
        Ident *name = static_cast<Ident*>(operand);
        Decl *proc_group = name->ref;
        ProcLit *procedure = find_procedure_overloaded(proc_group, ce->arguments);
        if (procedure) {
            proc_type = static_cast<ProcType*>(procedure->proc_type->type);
            is_valid = true;
        } else {
            TupleType *args_type = tuple_from_values(ce->arguments);
            String arg_type_string = string_from_type(args_type);
            if (proc_group->procedures.count == 1) {
                String param_type_string = string_from_type(static_cast<ProcType*>(proc_group->procedures[0]->type)->params);
                report_error(ce, "invalid argument type(s) '{}' for '{}' which takes '{}'", arg_type_string, name->name, param_type_string);
            } else {
                report_error(ce, "no procedure '{}' that accepts argument types '{}'", name->name, arg_type_string);
            }
        }
    } else {
        if (operand->type->kind == Type_Proc) {
            proc_type = static_cast<ProcType*>(operand->type);
            if (check_argument_procedure_types_assignable(ce->arguments, proc_type->params)) {
                is_valid = true;
            } else {
                TupleType *args_type = tuple_from_values(ce->arguments);
                String arg_type_string = string_from_type(args_type);
                String param_type_string = string_from_type(proc_type->params);
                report_error(ce, "invalid argument type(s) '{}', procedure takes '{}'", arg_type_string, param_type_string);
            }
        } else {
            report_error(operand, "operand does not evaluate to a procedure");
        }
    }

    if (is_valid) {
        ce->type = proc_type->results;
        ce->mode = AddressingMode_Value;
    } else {
        ce->type = t_invalid;
        ce->mode = AddressingMode_Invalid;
    }
}

void resolve_paren_expr(Resolver *R, ParenExpr *pe) {
    resolve_expr_base(R, pe->operand);

    pe->type = pe->operand->type;
    pe->mode = pe->operand->mode;
}

void resolve_block_expr(Resolver *R, BlockExpr *block) {
    Scope *scope = scope_create(R->scope, block);
    R->scope = scope;

    for (Ast *stmt : block->statements) {
        resolve_stmt(R, stmt);
    }

    Ast *resulting = block->trailing;
    if (resulting) {
        resolve_expr(R, resulting);
        block->mode = resulting->mode;
        block->type = resulting->type;
    } else {
        block->mode = AddressingMode_NoValue;
        block->type = t_void;
    }

    R->scope = scope->parent;
}

void resolve_array_expr(Resolver *R, ArrayExpr *array) {
    if (array->operand) {
        resolve_expr_base(R, array->operand);
    }

    for (Ast *elem : array->elems) {
        resolve_expr(R, elem);
    }

    Ast *operand = array->operand;

    bool is_literal = false;
    bool is_index = false;
    if (operand) {
        switch (operand->mode) {
            case AddressingMode_Value:
            case AddressingMode_Variable:
                is_index = true;
                is_literal = false;
                break;
            case AddressingMode_Type:
                is_literal = true;
                break;
            case AddressingMode_Procedure:
                report_error(operand, "procedure cannot be indexed");
                break;
            default:
                is_literal = true;
                break;
        }
    }

    if (is_literal) {
        Type *base_type = nullptr;
        if (operand) {
            base_type = operand->type;
        } else if (array->elems.count > 0) {
            base_type = array->elems[0]->type;
        }

        String base_type_string = string_from_type(base_type);

        for (Ast *elem : array->elems) {
            if (!types_assignable(base_type, elem->type)) {
                String elem_type_string = string_from_type(elem->type);
                report_error(elem, "invalid element of type '{}' in array of type '[]{}'", elem_type_string, base_type_string);
            }
        }

        array->type = array_type_create(base_type, nullptr, false);
        array->mode = AddressingMode_Variable;
    } else if (is_index) {
        if (!is_array_like_type(operand->type)) {
            String operand_type_string = string_from_type(operand->type);
            report_error(operand, "index operand of type '{}' is not an array or pointer", operand_type_string);
        }

        if (array->elems.count > 1) {
            report_error(array->operand, "multiple elements in index value");
        } else if (array->elems.count == 0) {
            report_error(array->operand, "missing index value");
        }
        Ast *index = array->elems[0];

        if (!is_integer_type(index->type)) {
            String index_type_string = string_from_type(index->type);
            report_error(index, "array index value is an invalid non-integer type '{}'", index_type_string);
        }

        array->type = operand->type->base;
        array->mode = AddressingMode_Variable;
    }
}

void resolve_compound_literal(Resolver *R, CompoundLiteralExpr *comp) {
    resolve_expr_base(R, comp->operand);

    if (comp->operand->mode == AddressingMode_Type) {
        report_error(comp->operand, "expected a type");
    }

    for (Ast *v : comp->initializer_list) {
        resolve_expr(R, v);
    }

    comp->type = comp->operand->type;
}

void resolve_if_expr(Resolver *R, IfExpr *if_expr) {
    Ast *condition = if_expr->condition;
    if (condition) {
        resolve_expr(R, condition);
    }

    Ast *then = if_expr->then_expr;
    IfExpr *elif = if_expr->else_if;

    resolve_expr(R, then);


    if_expr->mode = then->mode;
    if_expr->type = then->type;

    if (elif) {
        resolve_if_expr(R, elif);
        //@Note Check resulting values from if expressions
        if (then->mode != AddressingMode_NoValue || elif->mode != AddressingMode_NoValue) {
            if (!types_assignable(then->type, elif->type)) {
                String then_string = string_from_type(then->type);
                String elif_string = string_from_type(elif->type);
                report_error(if_expr, "if-chain, block trailing expressions are not same type '{}', '{}'.", then_string, elif_string);
                if_expr->mode = AddressingMode_Invalid;
                if_expr->type = t_invalid;
            }
        }
    }
}

void resolve_ifcase_expr(Resolver *R, IfCaseExpr *ifcase_expr) {
    resolve_expr(R, ifcase_expr->condition);

    resolve_expr(R, ifcase_expr->block);
}

void resolve_star_expr(Resolver *R, StarExpr *star, bool is_type) {
    Ast *operand = star->operand;

    if (is_type) {
        resolve_type(R, operand);
        star->type = pointer_type_create(operand->type);
        star->mode = AddressingMode_Type;
    } else {
        resolve_expr_base(R, operand);
        switch (operand->mode) {
            default:
                assert(0);
                star->type = t_invalid;
                star->mode = AddressingMode_Invalid;
                break;
            case AddressingMode_Value:
                report_error(operand, "operand is a temporary value");
                break;
            case AddressingMode_Type: {
                star->type = pointer_type_create(operand->type);
                star->mode = AddressingMode_Type;
                break;
            }
            case AddressingMode_Variable:
                star->type = pointer_type_create(operand->type);
                star->mode = AddressingMode_Variable;
                break;
        }
    }
}

void resolve_deref_expr(Resolver *R, DerefExpr *deref) {
    Ast *operand = deref->operand;
    resolve_expr(R, operand);

    if (is_pointer_type(operand->type)) {
        deref->mode = AddressingMode_Value;
        deref->type = deref_type(operand->type);
    } else {
        String op_type_string = string_from_type(operand->type);
        report_error(operand, "cannot dereference operand of type '{}'", op_type_string);
    }
}

void resolve_cast_expr(Resolver *R, CastExpr *cast_expr) {
    resolve_type(R, cast_expr->conversion_type);
    resolve_expr_base(R, cast_expr->operand);

    if (types_castable(cast_expr->operand->type, cast_expr->conversion_type->type)) {
        cast_expr->type = cast_expr->conversion_type->type;
        cast_expr->mode = AddressingMode_Value;
    } else {
        String op_type = string_from_type(cast_expr->operand->type);
        String conv_type = string_from_type(cast_expr->conversion_type->type);
        report_error(cast_expr, "cannot cast '{}' to '{}", op_type, conv_type);
        cast_expr->type = t_invalid;
    }
}

void resolve_param(Resolver *R, Param *param) {
    if (param->type_defn) {
        resolve_type(R, param->type_defn);
    }

    if (param->default_value) {
        resolve_expr_base(R, param->default_value);
    }

    if (param->type_defn && param->default_value) {
        if (!types_assignable(param->type_defn->type, param->default_value->type)) {
            report_error(param, "type mismatch for default parameter");
        }
    }

    if (param->type_defn) {
        param->type = param->type_defn->type;
    } else if (param->default_value) {
        param->type = param->default_value->type;
    }
}

void resolve_proc_type(Resolver *R, ProcTypeDefn *pt) {
    Array<Type*> params;
    for (Param *param : pt->params) {
        resolve_param(R, param);

        for (int i = 0; i < param->names.count; i++) {
            params.append(param->type_defn->type);
        }
    }

    for (Ast *ret : pt->results) {
        resolve_type(R, ret);
    }

    Type *results = nullptr;
    if (pt->results.count == 1) {
        results = pt->results[0]->type;
    } else if (pt->results.count > 1) {
        TupleType *tup = tuple_from_values(pt->results);
        results = tup;
    } else {
        results = t_void;
    }

    ProcType *proc_type = type_new<ProcType>();
    proc_type->params = type_new<TupleType>();
    proc_type->params->types = params;
    proc_type->results = results;
    pt->type = proc_type;
}

void resolve_array_type(Resolver *R, ArrayTypeDefn *array) {
    resolve_type(R, array->operand);

    if (array->size) {
        resolve_expr_base(R, array->size);
    }

    ArrayType *type = array_type_create(array->operand->type, array->size, array->dynamic);
    array->type = type;
}

void resolve_union_type(Resolver *R, UnionTypeDefn *ut) {
}

void resolve_enum_type(Resolver *R, EnumTypeDefn *et) {
}


void resolve_struct_type(Resolver *R, StructTypeDefn *type_defn) {
    StructType *st = ast_new<StructType>();
    type_defn->type = st;

    Scope *scope = scope_create(R->scope, type_defn);
    st->scope = scope;
    R->scope = scope;

    for (ValueDecl *member : type_defn->members) {
        //@Todo: Should struct members be resolved out of order?
        forward_declare_value_decl(R, scope, member, true);
        resolve_value_decl(R, member, true);

        // assert(member->type);

        for (int i = 0; i < member->names.count; i++) {
            Decl *decl = member->names[i]->ref;
            st->members.append(decl->type);
        }
    }

    R->scope = scope->parent;
}

void resolve_enumerator(Resolver *R, Enumerator *enumerator) {
}

void resolve_type(Resolver *R, Ast *expr) {
    switch (expr->kind) {
        default:
            assert(0);
            break;
        case Ast_Ident:
            resolve_name(R, (Ident *)expr);
            if (expr->mode != AddressingMode_Type) {
                report_error(expr, "expected a type");
            }
            break;

        case Ast_ProcType:
            resolve_proc_type(R, (ProcTypeDefn *)expr);
            break;

        case Ast_StarExpr:
            resolve_star_expr(R, (StarExpr *)expr, true);
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
    }
}

void resolve_proc_lit(Resolver *R, ProcLit *proc_lit) {
    resolve_proc_type(R, proc_lit->proc_type);

    Scope *scope = scope_create(R->scope, proc_lit);
    R->scope = scope;

    for (Param *param : proc_lit->proc_type->params) {
        for (Ident *name : param->names) {
            Decl *lookup = decl_find(scope, name->name);
            if (lookup) {
                report_redeclaration(name);
            } else {
                Decl *decl = decl_var_create(scope, name->name);
                decl->resolve_state = ResolveState_Completed;
                decl->node = param;
                name->ref = decl;
                decl->type = param->type;
            }
        }
    }

    if (proc_lit->body) {
        resolve_block_expr(R, proc_lit->body);
    }

    R->scope = scope->parent;
}

void resolve_expr(Resolver *R, Ast *expr) {
    resolve_expr_base(R, expr);

    if (expr->mode == AddressingMode_Type) {
        report_error(expr, "operand is a type, not a value.");
    }
}

void resolve_expr_base(Resolver *R, Ast *expr) {
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
        case Ast_CallExpr:
            resolve_call_expr(R, (CallExpr *)expr);
            break;
        case Ast_ParenExpr:
            resolve_paren_expr(R, (ParenExpr *)expr);
            break;
        case Ast_BlockExpr:
            resolve_block_expr(R, (BlockExpr *)expr);
            break;
        case Ast_ArrayExpr:
            resolve_array_expr(R, (ArrayExpr *)expr);
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
            resolve_star_expr(R, (StarExpr *)expr, false);
            break;
        case Ast_DerefExpr:
            resolve_deref_expr(R, (DerefExpr *)expr);
            break;
        case Ast_CastExpr:
            resolve_cast_expr(R, (CastExpr *)expr);
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

void resolve_value_decl_stmt(Resolver *R, ValueDecl *vd) {
    Scope *scope = R->scope;

    forward_declare_value_decl(R, scope, vd, false);

    resolve_value_decl(R, vd, false);
}

void resolve_value_decl(Resolver *R, ValueDecl *vd, bool is_global) {
    Scope *scope = R->scope;

    if (is_global) {
        if (vd->is_mutable) {
            if (vd->names.count < vd->values.count) {
                report_error(vd, "too many values on right hand side");
            } else {
            }
        } else {
            if (vd->names.count != vd->values.count) {
                report_error(vd, "number of right hand values does not match number of left side, in constant value declaration");
            }
        }

        for (int i = 0; i < vd->names.count; i++) {
            Ident *name = vd->names[i];
            Decl *decl = name->ref;
            resolve_decl(R, decl);
        }
    } else {
        if (vd->is_mutable) {
            if (vd->type_defn) {
                resolve_expr_base(R, vd->type_defn);
            }

            for (Ast *expr : vd->values) {
                resolve_expr_base(R, expr);
            }

            if (vd->type_defn) {
                for (Ast *rhs : vd->values) {
                    if (!types_assignable(vd->type_defn->type, rhs->type)) {
                        report_error(rhs, "right hand side of declaration does not match specified type");
                    }
                }
            }

            int value_count = type_arity(vd->values);
            if (value_count == vd->names.count) {
                for (int i = 0; i < value_count; i++) {
                    for (int name_idx = 0, val_idx = 0; val_idx < value_count; val_idx++) {
                        Ast *value = value = vd->values[val_idx];
                        if (is_tuple_type(value->type)) {
                            TupleType *tup = static_cast<TupleType*>(value->type);
                            for (int ti = 0; ti < tup->types.count; ti++) {
                                Ident *name = vd->names[name_idx];
                                Decl *decl = name->ref;
                                decl->resolve_state = ResolveState_Completed;
                                Type *type = tup->types[ti];
                                decl->type = type;
                                name->type = type;
                                name_idx++;
                            }
                        } else {
                            Ident *name = vd->names[name_idx];
                            Decl *decl = name->ref;
                            decl->resolve_state = ResolveState_Completed;
                            Type *type = value->type;
                            decl->type = type;
                            name->type = type;
                            name_idx++;
                        }
                    }
                }
            } else if (value_count > 0) {
                report_error(vd, "mismatched number of values and names in declaration");
            }
        } else {
            for (Ident *name : vd->names) {
                Decl *decl = name->ref;
                resolve_decl(R, decl);
                name->type = decl->type;
            }
        }
    }
}

void resolve_assign_stmt(Resolver *R, AssignStmt *assign) {
    for (Ast *lhs : assign->lhs) {
        resolve_expr_base(R, lhs);

        if (lhs->mode != AddressingMode_Variable) {
            report_error(lhs, "cannot assign, not an l-value");
        }
    }

    for (Ast *rhs : assign->rhs) {
        resolve_expr_base(R, rhs);
    }

    int value_count = type_arity(assign->rhs);
    if (value_count == assign->lhs.count) {
        for (int i = 0; i < value_count; i++) {
            for (int dst_idx = 0, val_idx = 0; val_idx < value_count; val_idx++) {
                Ast *value = value = assign->rhs[val_idx];
                if (is_tuple_type(value->type)) {
                    TupleType *tup = static_cast<TupleType*>(value->type);
                    for (int ti = 0; ti < tup->types.count; ti++) {
                        Ast *dst = assign->lhs[dst_idx];
                        Type *type = tup->types[ti];

                        if (!types_assignable(dst->type, type)) {
                            String val_t_string = string_from_type(type);
                            String dst_t_string = string_from_type(dst->type);
                            report_error(dst, "bad type of '{}' in assignment, expected '{}'", val_t_string, dst_t_string);
                        }
                        dst_idx++;
                    }
                } else {
                    Ast *dst = assign->lhs[dst_idx];
                    Type *type = value->type;
                    if (!types_assignable(dst->type, type)) {
                        String val_t_string = string_from_type(type);
                        String dst_t_string = string_from_type(dst->type);
                        report_error(dst, "bad type of '{}' in assignment, expected '{}'", val_t_string, dst_t_string);
                    }
                    dst_idx++;
                }
            }
        }
    } else {
        report_error(assign, "mismatched number of values in assignment");
    }
}

void resolve_break_stmt(Resolver *R, BreakStmt *break_stmt) {
    if (break_stmt->operand) {
        resolve_expr_base(R, break_stmt->operand);
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

    ProcLit *proc_lit = static_cast<ProcLit*>(control);

    for (Ast *result : return_stmt->results) {
        resolve_expr_base(R, result);
    }

    Type *ret_type = nullptr;
    if (return_stmt->results.count > 1) {
        TupleType *tup = tuple_from_values(return_stmt->results);
        ret_type = tup;
    } else if (return_stmt->results.count == 1) {
        ret_type = return_stmt->results[0]->type;
    }

    if (proc_lit) {
        ProcType *proc_type = static_cast<ProcType*>(proc_lit->proc_type->type);
        if (!types_assignable(ret_type, proc_type->results)) {
            String ret_type_str = string_from_type(ret_type);
            String result_type_str = string_from_type(proc_type->results);
            report_error(return_stmt, "mismatched types, expected '{}' return, got '{}'", result_type_str, ret_type_str);
        }
    }
}

void resolve_for_stmt(Resolver *R, ForStmt *for_stmt) {
    Scope *scope = scope_create(R->scope, for_stmt);
    R->scope = scope;

    if (for_stmt->init) {
        resolve_stmt(R, for_stmt->init);
    }

    if (for_stmt->condition) {
        resolve_expr(R, for_stmt->condition);
    }

    if (for_stmt->post) {
        resolve_stmt(R, for_stmt->post);
    }

    resolve_expr_base(R, for_stmt->block);

    R->scope = scope->parent;
}

void resolve_case_expr(Resolver *R, CaseExpr *case_expr) {
    if (case_expr->operand) {
        resolve_expr_base(R, case_expr->operand);
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
            resolve_value_decl_stmt(R, (ValueDecl *)stmt);
            break;
        case Ast_Assign:
            resolve_assign_stmt(R, (AssignStmt *)stmt);
            break;
        case Ast_ExprStmt:
            resolve_expr_base(R, ((ExprStmt *)stmt)->expr);
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

void declare_builtin_types(Resolver *R) {
    Scope *scope = R->global_scope;
    for (TypeKind kind = Type_Void; kind <= Type_String; kind = (TypeKind)(kind + 1)) {
        Type *type = &g_builtin_types[kind];
        Atom *name = atom_create(type->name);
        Decl *decl = decl_type_create(scope, name, nullptr);
        decl->kind = Decl_Type;
        decl->resolve_state = ResolveState_Completed;
        decl->type = type;
    }
}

template<>
struct std::formatter<DeclKind> : std::formatter<string_view> {
    auto format(DeclKind kind, format_context& ctx) const {
        string_view name = "";
        switch (kind) {
            default: name = "nil"; break;
            case Decl_Constant:  name = "Constant"; break;
            case Decl_Var:       name = "Var"; break;
            case Decl_Type:      name = "Type"; break;
            case Decl_ProcGroup: name = "ProcGroup"; break;
            case Decl_Proc:      name = "Proc"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};

void resolve_program(Resolver *R, Parser *P) {
    Scope *global_scope = scope_create(nullptr, nullptr);
    R->global_scope = global_scope;
    R->scope = global_scope;

    declare_builtin_types(R);

    add_global_constant_int(R, STRZ("true"), 1);
    add_global_constant_int(R, STRZ("false"), 0);

    for (AstFile *file : R->files) {
        for (Ast *node : file->decls) {
            if (node->kind == Ast_ValueDecl) {
                forward_declare_value_decl(R, global_scope, static_cast<ValueDecl*>(node), true);
            }
        }
    }

    if (0) {
        for (Decl *decl : global_scope->decl_table) {
            std::println("DECL '{}' {}", decl->name, decl->kind);
        }
    }

    for (AstFile *file : R->files) {
        for (Ast *stmt : file->decls) {
            resolve_top_level_stmt(R, stmt);
        }
    }

}
