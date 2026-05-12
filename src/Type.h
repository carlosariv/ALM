#pragma once

enum TypeKind {
    Type_Unknown,

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

    Type_Pointer,
    Type_Array,
    Type_Any,
    Type_Enum,
    Type_Struct,
    Type_Union,
    Type_Procedure,
    Type_Tuple,
    Type_String,

    Type_COUNT
};

struct Type {
    TypeKind kind = Type_Unknown;
};

struct TypePointer : Type {
};
