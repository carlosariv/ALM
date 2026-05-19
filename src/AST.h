#pragma once

#include "BaseTypes.h"
#include "Array.h"
#include "Token.h"

struct Ast;
struct Type;
struct Symbol;
struct BlockExpr;

enum AstKind {
    Ast_Unknown,
    Ast_Error,

    Ast_File,

    Ast_ValueDecl,
    Ast_Assign,
    Ast_ExprStmt,
    Ast_EmptyStmt,
    Ast_Break,
    Ast_Continue,
    Ast_Fallthrough,
    Ast_Return,
    Ast_Case,
    Ast_Do,
    Ast_While,
    Ast_For,

    Ast_Ident,
    Ast_LiteralExpr,
    Ast_UnaryExpr,
    Ast_BinaryExpr,
    Ast_SelectorExpr,
    Ast_SubscriptExpr,
    Ast_CallExpr,
    Ast_ParenExpr,
    Ast_BlockExpr,
    Ast_CompoundLiteral,
    Ast_IfExpr,
    Ast_IfCaseExpr,
    Ast_StarExpr,
    Ast_DerefExpr,

    Ast_ProcType,
    Ast_ProcLit,
    Ast_Param,
    Ast_ArrayType,
    Ast_StructType,
    Ast_UnionType,
    Ast_EnumType,
    Ast_Enumerator,

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

struct Ident : Ast {
    Atom *name;
    Token token;
    Symbol *symbol = nullptr;

    Ident() {
        kind = Ast_Ident;
    }
};

struct ValueDecl : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Ast *type_defn;
    bool is_constant;

    ValueDecl() {
        kind = Ast_ValueDecl;
    }
};

struct AssignStmt : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Token token;
    Operator op;

    AssignStmt() {
        kind = Ast_Assign;
    }
};

struct Param : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Ast *type_defn;
    Param() {
        kind = Ast_Param;
    }
};

enum LiteralKind {
    Literal_Integer,
    Literal_Floating,
    Literal_String
};

struct LiteralExpr : Ast {
    LiteralKind literal_kind;
    union {
        String string_value;
        u64 integer_value;
        f64 float_value;
    };
    Token token;

    LiteralExpr() {
        kind = Ast_LiteralExpr;
    }
};

struct UnaryExpr : Ast {
    Operator op;
    Ast *operand;
    Token token;

    UnaryExpr() {
        kind = Ast_UnaryExpr;
    }
};

struct BinaryExpr : Ast {
    Operator op;
    Ast *lhs;
    Ast *rhs;
    Token token;

    BinaryExpr() {
        kind = Ast_BinaryExpr;
    }
};

struct CompoundLiteralExpr : Ast {
    Ast *operand;
    Array<Ast*> initializer_list;
    Token open;
    Token close;

    CompoundLiteralExpr() {
        kind = Ast_CompoundLiteral;
    }
};

struct SelectorExpr : Ast {
    Ast *operand;
    Ident *name;
    Token token;

    SelectorExpr() {
        kind = Ast_SelectorExpr;
    }
};

struct SubscriptExpr : Ast {
    Ast *operand;
    Ast *value;
    Token open;
    Token close;

    SubscriptExpr() {
        kind = Ast_SubscriptExpr;
    }
};

struct CallExpr : Ast {
    Ast *operand;
    Array<Ast*> arguments;
    Token open;
    Token close;

    CallExpr() {
        kind = Ast_CallExpr;
    }
};

struct IfExpr : Ast {
    Ast *condition;
    Ast *then_expr = nullptr;
    IfExpr *prev_if = nullptr;
    IfExpr *else_if = nullptr;
    bool is_final = false;
    Token token;

    IfExpr() {
        kind = Ast_IfExpr;
    }
};

struct IfCaseExpr : Ast {
    Ast *condition;
    BlockExpr *block;
    Token token;
    IfCaseExpr() {
        kind = Ast_IfCaseExpr;
    }
};

struct ParenExpr : Ast {
    Ast *expr;

    Token open;
    Token close;

    ParenExpr() {
        kind = Ast_ParenExpr;
    }
};

struct BlockExpr : Ast {
    Array<Ast*> statements;
    bool is_ifcase = false;
    Ast *trailing = nullptr;
    Token open;
    Token close;

    BlockExpr() {
        kind = Ast_BlockExpr;
    }
};

struct StarExpr : Ast {
    Ast *elem;
    Token token;

    StarExpr() {
        kind = Ast_StarExpr;
    }
};

struct DerefExpr : Ast {
    Ast *operand;
    Token token;

    DerefExpr() {
        kind = Ast_DerefExpr;
    }
 };

struct EmptyStmt : Ast {
    Token token;
    EmptyStmt() {
        kind = Ast_EmptyStmt;
    }
};

struct ExprStmt : Ast {
    Ast *expr;
    ExprStmt() {
        kind = Ast_ExprStmt;
    }
};

struct CaseExpr : Ast {
    Ast *expr;
    bool is_default;
    Array<Ast*> statements;
    CaseExpr *prev_clause;
    CaseExpr *next_clause;
    Token token;

    CaseExpr() {
        kind = Ast_Case;
    }
};

struct WhileStmt : Ast {
    Ast *condition;
    BlockExpr *block;
    Token token;

    WhileStmt() {
        kind = Ast_While;
    }
};

struct DoStmt : Ast {
    Ast *condition;
    BlockExpr *block;
    Token token;

    DoStmt() {
        kind = Ast_Do;
    }
};

struct ForStmt : Ast {
    Ast *condition;
    BlockExpr *block;
    Token token;

    ForStmt() {
        kind = Ast_For;
    }
};

struct ContinueStmt : Ast {
    Token token;

    ContinueStmt() {
        kind = Ast_Continue;
    }
};

struct BreakStmt : Ast {
    Token token;

    BreakStmt() {
        kind = Ast_Break;
    }
};

struct FallthroughStmt : Ast {
    Token token;

    FallthroughStmt() {
        kind = Ast_Fallthrough;
    }
};


struct ReturnStmt : Ast {
    Ast *expr;
    Token token;

    ReturnStmt() {
        kind = Ast_Return;
    }
};

struct ProcTypeDefn : Ast {
    Array<Param*> params;
    Ast *return_type;
    Token open;
    Token close;

    ProcTypeDefn() {
        kind = Ast_ProcType;
    }
};

struct ArrayTypeDefn : Ast {
    Ast *elem;
    Ast *size;
    bool dynamic;
    Token open;
    Token close;

    ArrayTypeDefn() {
        kind = Ast_ArrayType;
    }
};

struct StructTypeDefn : Ast {
    Array<Ast*> members;
    Token token;
    Token open;
    Token close;

    StructTypeDefn() {
        kind = Ast_StructType;
    }
};

struct UnionTypeDefn : Ast {
    Array<Ast*> members;
    Token token;
    Token open;
    Token close;

    UnionTypeDefn() {
        kind = Ast_UnionType;
    }
};

struct EnumTypeDefn : Ast {
    Array<Ast*> members;
    Token token;
    Token open;
    Token close;

    EnumTypeDefn() {
        kind = Ast_EnumType;
    }
};

struct Enumerator : Ast {
    Ident *name;
    Ast *value;

    Enumerator() {
        kind = Ast_Enumerator;
    }
};

struct ProcLit : Ast {
    ProcTypeDefn *proc_type;
    Ast *body;

    ProcLit() {
        kind = Ast_ProcLit;
    }
};

void *ast_alloc(int bytes);

template <typename T>
T *ast_new() {
    T *node = (T* )ast_alloc(sizeof(T));
    *node = T();
    return node;
}

void ast_print(Ast *node);
String string_from_token(TokenKind token);
String string_from_operator(Operator op);
