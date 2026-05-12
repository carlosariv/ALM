#pragma once

#include <vector>

#include "BaseTypes.h"
#include "Array.h"
#include "Token.h"

struct Ast;
struct Type;

enum Operator {
    Operator_Nil,

    Operator_UnaryPlus,
    Operator_Negate,
    Operator_Not,
    Operator_AddressOf,
    Operator_IndexOf,
    Operator_Deref,
    Operator_Cast,

    Operator_Add,
    Operator_Sub,
    Operator_Mult,
    Operator_Div,
    Operator_Mod,

    Operator_Equal,
    Operator_Less,
    Operator_Greater,
    Operator_LessEqual,
    Operator_GreaterEqual,
    Operator_And,
    Operator_Or,

    Operator_LeftShift,
    Operator_RightShift,
    Operator_Xor,
    Operator_BitwiseAnd,
    Operator_BitwiseOr,

    Operator_COUNT
};

enum LiteralKind {
    Literal_Integer,
    Literal_Floating,
    Literal_String
};

enum AstKind {
    Ast_Unknown,
    Ast_Error,

    Ast_File,
    Ast_Name,
    Ast_Param,
    Ast_ValueDecl,

    Ast_EmptyStmt,
    Ast_ExprStmt,
    Ast_Assign,
    Ast_Return,
    Ast_While,
    Ast_For,

    Ast_LiteralExpr,
    Ast_UnaryExpr,
    Ast_BinaryExpr,
    Ast_IndexExpr,
    Ast_SubscriptExpr,
    Ast_CallExpr,
    Ast_ParenExpr,
    Ast_BlockExpr,
    Ast_CompoundLiteral,
    Ast_IfExpr,

    Ast_PointerType,
    Ast_ArrayType,
    Ast_StructType,
    Ast_UnionType,
    Ast_EnumType,
    Ast_Enumerator,
    Ast_ProcType,
    Ast_ProcLit,

    Ast_COUNT
};

struct Ast {
    AstKind kind = Ast_Error;
    Type *type = nullptr;
};

struct AstError : Ast {
    AstError() {
        kind = Ast_Error;
    }
};

struct AstFile : Ast {
    Array<Ast*> decls;

    AstFile() {
        kind = Ast_File;
    }
};

struct AstName : Ast {
    Atom *name;
    Token token;
    AstName() {
        kind = Ast_Name;
    }
};

struct AstProcLit : Ast {
    Array<Ast*> params;
    Ast *return_type;
    Ast *body;

    AstProcLit() {
        kind = Ast_ProcLit;
    }
};

struct AstValueDecl : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Ast *type_defn;
    bool constant;

    AstValueDecl() {
        kind = Ast_ValueDecl;
    }
};

struct AstParam : Ast {
    Atom *name;
    Ast *type_defn;
    Ast *init;
    AstParam() {
        kind = Ast_Param;
    }
};

struct AstLiteralExpr : Ast {
    LiteralKind literal_kind;
    union {
        String string_value;
        u64 integer_value;
        f64 float_value;
    };
    Token token;

    AstLiteralExpr() {
        kind = Ast_LiteralExpr;
    }
};

struct AstUnaryExpr : Ast {
    Operator op;
    Ast *operand;
    Token token;

    AstUnaryExpr() {
        kind = Ast_UnaryExpr;
    }
};

struct AstBinaryExpr : Ast {
    Operator op;
    Ast *lhs;
    Ast *rhs;

    AstBinaryExpr() {
        kind = Ast_BinaryExpr;
    }
};

struct AstCompoundLiteral : Ast {
    Ast *operand;
    Array<Ast*> initializer_list;
    Token open;
    Token close;

    AstCompoundLiteral() {
        kind = Ast_CompoundLiteral;
    }
};

struct AstIndexExpr : Ast {
    Ast *lhs;
    Ast *elem;

    AstIndexExpr() {
        kind = Ast_IndexExpr;
    }
};

struct AstSubscriptExpr : Ast {
    Ast *operand;
    Ast *value;
    Token open;
    Token close;

    AstSubscriptExpr() {
        kind = Ast_SubscriptExpr;
    }
};

struct AstCallExpr : Ast {
    Ast *operand;
    Array<Ast*> arguments;

    AstCallExpr() {
        kind = Ast_CallExpr;
    }
};

struct AstIfExpr : Ast {
    Ast *condition;
    Ast *then_expr;
    AstIfExpr *else_if;

    AstIfExpr() {
        kind = Ast_IfExpr;
    }
};

struct AstParenExpr : Ast {
    Ast *expr;

    Token open;
    Token close;

    AstParenExpr() {
        kind = Ast_ParenExpr;
    }
};

struct AstBlockExpr : Ast {
    Array<Ast*> statements;

    AstBlockExpr() {
        kind = Ast_BlockExpr;
    }
};

struct AstEmptyStmt : Ast {
    AstEmptyStmt() {
        kind = Ast_EmptyStmt;
    }
};

struct AstExprStmt : Ast {
    Ast *expr;
    AstExprStmt() {
        kind = Ast_ExprStmt;
    }
};

struct AstAssign : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;

    AstAssign() {
        kind = Ast_Assign;
    }
};

struct AstReturn : Ast {
    Ast *expr;

    AstReturn() {
        kind = Ast_Return;
    }
};

struct AstPointerType : Ast {
    Ast *elem;
    AstPointerType() {
        kind = Ast_PointerType;
    }
};

struct AstArrayType : Ast {
    Ast *elem;
    Ast *size;
    bool dynamic;

    AstArrayType() {
        kind = Ast_ArrayType;
    }
};

struct AstStructType : Ast {
    Array<Ast*> members;

    Token token;
    Token open;
    Token close;

    AstStructType() {
        kind = Ast_StructType;
    }
};

struct AstUnionType : Ast {
    Array<Ast*> members;

    AstUnionType() {
        kind = Ast_UnionType;
    }
};

struct AstEnumType : Ast {
    Array<Ast*> members;

    Token token;
    Token open;
    Token close;

    AstEnumType() {
        kind = Ast_EnumType;
    }
};

struct Enumerator : Ast {
    AstName *name;
    Ast *value;

    Enumerator() {
        kind = Ast_Enumerator;
    }
};

struct AstProcType : Ast {
    Array<Ast*> params;
    Ast *return_type;
    AstProcType() {
        kind = Ast_ProcType;
    }
};

void ast_print(Ast *node);

void *ast_alloc(int bytes);

template <typename T>
T *ast_new() {
    T *node = (T* )ast_alloc(sizeof(T));
    *node = T();
    return node;
}


String string_from_token(TokenKind token);
String string_from_operator(Operator op);

