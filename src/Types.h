#pragma once

#include "Array.h"
#include "String.h"

struct TupleType;

enum TypeKind {
    Type_Unknown,
    Type_BuiltinBegin,
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
    Type_BuiltinEnd,

    Type_String,
    Type_Pointer,
    Type_Array,
    Type_Any,
    Type_Enum,
    Type_Struct,
    Type_Union,
    Type_Proc,
    Type_Tuple,

    Type_COUNT
};

struct Type {
    TypeKind kind = Type_Unknown;
    String name;
    int bytes;
};

struct PointerType : Type {
    Type *base;

    PointerType() {
        kind = Type_Pointer;
    }
};

struct ArrayType : Type {
    Type *base;
    // CompileTimeValue fixed_size;

    ArrayType() {
        kind = Type_Array;
    }
};

struct StructType : Type {
    Array<Type*> members;

    StructType() {
        kind = Type_Struct;
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

void *type_alloc(int bytes);

template <typename T>
T *type_new() {
    T *type = (T *)type_alloc(sizeof(T));
    return type;
}

bool type_match(Type *lhs, Type *rhs);
int get_type_arity(Type *type);
PointerType *pointer_type_create(Type *t);
