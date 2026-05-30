#pragma once

#include "Array.h"
#include "String.h"

struct TupleType;
struct Ast;
struct Scope;

enum TypeKind {
    Type_Unknown,
    Type_Invalid,
    Type_Void,
    Type_Bool,
    Type_U8,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_I8,
    Type_I16,
    Type_I32,
    Type_I64,
    Type_F32,
    Type_F64,
    Type_String,

    Type_Pointer,
    Type_Array,
    Type_Enum,
    Type_Struct,
    Type_Union,
    Type_Proc,
    Type_Tuple,
    Type_Any,

    Type_COUNT
};

struct Type {
    TypeKind kind = Type_Unknown;
    String name;
    int bytes;
    Type *base = nullptr;
};

struct PointerType : Type {
    PointerType() {
        kind = Type_Pointer;
    }
};

struct ArrayType : Type {
    // Ast *size;
    bool is_dynamic = false;
    // CompileTimeValue fixed_size;

    ArrayType() {
        kind = Type_Array;
    }
};

struct StructType : Type {
    Array<Type*> members;
    Scope *scope = nullptr;

    StructType() {
        kind = Type_Struct;
    }
};

struct UnionType : Type {
    Array<Type*> members;
    Scope *scope = nullptr;

    UnionType() {
        kind = Type_Union;
    }
};



struct EnumType : Type {
    Scope *scope = nullptr;

    EnumType() {
        kind =  Type_Enum;
    }
};

struct ProcType : Type {
    TupleType *params;
    Type *results;

    ProcType() {
        kind = Type_Proc;
    }
};

struct TupleType : Type {
    Array<Type*> types;
    TupleType() {
        kind = Type_Tuple;
    }
};

extern Type g_builtin_types[];
extern Type *t_void;
extern Type *t_bool;
extern Type *t_u8;
extern Type *t_u16;
extern Type *t_u32;
extern Type *t_u64;
extern Type *t_i8;
extern Type *t_i16;
extern Type *t_i32;
extern Type *t_i64;
extern Type *t_f32;
extern Type *t_f64;
extern Type *t_string;
extern Type *t_invalid;

void *type_alloc(int bytes);

template <typename T>
T *type_new() {
    T *type = (T *)type_alloc(sizeof(T));
    *type = T();
    return type;
}

int type_arity(Type *type);
bool types_assignable(Type *lhs, Type *rhs);
bool types_castable(Type *dst, Type *src);

PointerType *pointer_type_create(Type *t);
ArrayType *array_type_create(Type *base, Ast *size, bool is_dynamic);

String string_from_type(Type *type);

Type *deref_type(Type *type);

inline bool is_user_defined_type(Type *type) {
    switch (type->kind) {
        case Type_Struct:
        case Type_Enum:
        case Type_Union:
            return true;
        default:
            return false;
    }
}

inline bool is_pointer_type(Type *type) {
    return type->kind == Type_Pointer;
}

inline bool is_array_type(Type *type) {
    return type->kind == Type_Array;
}

inline bool is_string_type(Type *type) {
    return type->kind == Type_String;
}

inline bool is_proc_type(Type *type) {
    return type->kind == Type_Proc;
}

inline bool is_tuple_type(Type *type) {
    return type->kind == Type_Tuple;
}

inline bool is_array_like_type(Type *type) {
    switch (type->kind) {
        default:
            return false;
        case Type_Array:
        case Type_Pointer:
            return true;
    }
}

inline bool is_integer_type(Type *type) {
    switch (type->kind) {
        default:
            return false;
        case Type_Bool:
        case Type_U8:
        case Type_U16:
        case Type_U32:
        case Type_U64:
        case Type_I8:
        case Type_I16:
        case Type_I32:
        case Type_I64:
        case Type_Enum:
            return true;
    }
}

inline bool is_numeric_type(Type *type) {
    switch (type->kind) {
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
        case Type_Enum:
            return true;
        default:
            return false;
    }
}

inline bool is_arithmetic_type(Type *type) {
    switch (type->kind) {
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
        case Type_Enum:
            return true;
        default:
            return false;
    }
}

inline bool is_signed_type(Type *type) {
    switch (type->kind) {
        case Type_I8:
        case Type_I16:
        case Type_I32:
        case Type_I64:
            return true;
        default:
            return false;
    }
}

inline Type *get_signed_type(Type *type) {
    switch (type->kind) {
        default:
            return type;
        case Type_U8:
            return t_i8;
        case Type_U16:
            return t_i16;
        case Type_U32:
            return t_i32;
        case Type_U64:
            return t_i64;
    }
}
