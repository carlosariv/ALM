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

bool type_match(Type *lhs, Type *rhs) {
    if (lhs == rhs) return true;
    return false;
}

int get_type_arity(Type *type) {
    switch (type->kind) {
        default:
            return 1;
        case Type_Void:
            return 0;
        case Type_Tuple: {
            int count = 0;
            TupleType *tup = static_cast<TupleType*>(type);
            for (int i = 0; i < tup->types.count; i++) {
                count += get_type_arity(tup->types[i]);
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
