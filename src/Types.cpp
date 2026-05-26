#include <assert.h>
#include "Types.h"

Type g_builtin_types[] = {
    {Type_Unknown, STRZ("<unknown>"), 0},
    {Type_Void,    STRZ("void"),      0},
    {Type_Bool,    STRZ("bool"),      1},
    {Type_U8,      STRZ("u8"),        1},
    {Type_U16,     STRZ("u16"),       2},
    {Type_U32,     STRZ("u32"),       4},
    {Type_U64,     STRZ("u64"),       8},
    {Type_I8,      STRZ("i8"),        1},
    {Type_I16,     STRZ("i16"),       2},
    {Type_I32,     STRZ("i32"),       4},
    {Type_I64,     STRZ("i64"),       8},
    {Type_F32,     STRZ("f32"),       4},
    {Type_F64,     STRZ("f64"),       8},
};

Type *t_void = &g_builtin_types[Type_Void];
Type *t_bool = &g_builtin_types[Type_Bool];
Type *t_u8 = &g_builtin_types[Type_U8];
Type *t_u16 = &g_builtin_types[Type_U16];
Type *t_u32 = &g_builtin_types[Type_U32];
Type *t_u64 = &g_builtin_types[Type_U64];
Type *t_i8 = &g_builtin_types[Type_I8];
Type *t_i16 = &g_builtin_types[Type_I16];
Type *t_i32 = &g_builtin_types[Type_I32];
Type *t_i64 = &g_builtin_types[Type_I64];
Type *t_f32 = &g_builtin_types[Type_F32];
Type *t_f64 = &g_builtin_types[Type_F64];
Type *t_string = &g_builtin_types[Type_String];

void *type_alloc(int bytes) {
    void *mem = malloc(bytes);
    return mem;
}



//@Note: Type Equality rules
// User defined types have to point to same distinct type such as structs, unions, enums, and procs
// Procedure types have to match signature, params and results
// Builtin types have to be identical
// Tuple subtypes must match and equal arity. Tuples can equal non-tuples as tuples can be a single value.

bool types_equal(Type *lhs, Type *rhs) {
    assert(lhs && rhs);
    if (lhs == rhs) return true;

    if (is_user_defined_type(lhs) || is_user_defined_type(rhs)) {
        return false;
    }

    if (is_pointer_type(lhs) != is_pointer_type(rhs)) {
        return false;
    } else if (is_pointer_type(lhs)) {
        return types_equal(lhs->base, rhs->base);
    }

    if (is_array_type(lhs) != is_array_type(rhs)) {
        return false;
    } else if (is_array_type(lhs)) {
        return types_equal(lhs->base, rhs->base);
    }

    if (is_proc_type(lhs) != is_proc_type(rhs)) {
        return false;
    } else if (is_proc_type(lhs)) {
        ProcType *l = static_cast<ProcType*>(lhs);
        ProcType *r = static_cast<ProcType*>(rhs);

        return types_equal(l->params, r->params) && types_equal(l->results, r->results);
    }

    if (is_tuple_type(lhs) || is_tuple_type(rhs)) {
        int left_arity = type_arity(lhs);
        int right_arity = type_arity(rhs);
        if (left_arity != right_arity) return false;

        if (left_arity == 1) {
            Type *l = lhs, *r = rhs;
            if (is_tuple_type(lhs)) {
                l = static_cast<TupleType*>(lhs)->types[0];
            }
            if (is_tuple_type(rhs)) {
                r = static_cast<TupleType*>(rhs)->types[0];
            }

            return types_equal(l, r);
        } else {
            assert(is_tuple_type(lhs) && is_tuple_type(rhs));
            TupleType *tl = static_cast<TupleType*>(lhs);
            TupleType *tr = static_cast<TupleType*>(rhs);
            for (int i = 0; i < tl->types.count; i++) {
                if (!types_equal(tl->types[i], tr->types[i])) {
                    return false;
                }
            }
            return true;
        }
    }

    if (lhs->kind == Type_String && rhs->kind == Type_String) return true;

    return false;
}

int type_arity(Type *type) {
    switch (type->kind) {
        default:
            return 1;
        case Type_Void:
            return 0;
        case Type_Tuple: {
            int count = 0;
            TupleType *tup = static_cast<TupleType*>(type);
            for (int i = 0; i < tup->types.count; i++) {
                count += type_arity(tup->types[i]);
            }
            return count;
        }
    }
}

PointerType *pointer_type_create(Type *base) {
    PointerType *pointer_type = type_new<PointerType>();
    pointer_type->base = base;
    return pointer_type;
}

ArrayType *array_type_create(Type *base, Ast *size, bool is_dynamic) {
    ArrayType *array_type = type_new<ArrayType>();
    array_type->base = base;
    // array_type->size = size;
    array_type->is_dynamic = is_dynamic;
    return array_type;
}

String string_from_type(Type *type) {
    String s;
    for (;;) {
        if (type == nullptr) return s;

        switch (type->kind) {
            default:
                break;
            case Type_Unknown:
                s = string_concat(s, STRZ("<unknown>"));
                break;
            case Type_Void:
            case Type_Bool:
            case Type_U8:
            case Type_U16:
            case Type_U32:
            case Type_U64:
            case Type_I8:
            case Type_I16:
            case Type_I32:
            case Type_I64:
            case Type_F32:
            case Type_F64:
            case Type_String:
                s = string_concat(s, type->name);
                break;

            case Type_Pointer:
                s = string_concat(s, STRZ("*"));
                break;
            case Type_Array:
                s = string_concat(s, STRZ("[]"));
                break;
            case Type_Any:
                s = string_concat(s, STRZ("any"));
                break;
            case Type_Enum:
                s = string_concat(s, STRZ("enum"));
                break;
            case Type_Struct:
                s = string_concat(s, STRZ("struct"));
                break;
            case Type_Union:
                s = string_concat(s, STRZ("union"));
                break;
            case Type_Proc:
                s = string_concat(s, STRZ("proc"));
                break;
            case Type_Tuple: {
                TupleType *tup = static_cast<TupleType*>(type);
                s = string_concat(s, STRZ("("));
                for (int i = 0; i < tup->types.count; i++) {
                    s = string_concat(s, string_from_type(tup->types[i]));
                    if (i < tup->types.count - 1) s = string_concat(s, STRZ(" "));
                }
                s = string_concat(s, STRZ(")"));
                break;
            }
        }
        type = type->base;
    }
}
