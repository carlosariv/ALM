#pragma once

#include <vector>

#include "BaseTypes.h"
#include "Array.h"
#include "Token.h"

struct Ast;
struct Type;
struct Symbol;

enum LiteralKind {
    Literal_Integer,
    Literal_Floating,
    Literal_String
};

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

    Ast_Name,
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
    Ast_StarExpr,

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

struct AstName : Ast {
    Atom *name;
    Token token;
    Symbol *symbol = nullptr;

    AstName() {
        kind = Ast_Name;
    }
};

struct AstValueDecl : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Ast *type_defn;
    bool is_constant;

    AstValueDecl() {
        kind = Ast_ValueDecl;
    }
};

struct AstAssign : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Token token;
    Operator op;

    AstAssign() {
        kind = Ast_Assign;
    }
};

struct AstParam : Ast {
    Array<Ast*> lhs;
    Array<Ast*> rhs;
    Ast *type_defn;
    AstParam() {
        kind = Ast_Param;
    }
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
    Token token;

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

struct AstSelectorExpr : Ast {
    Ast *operand;
    AstName *name;
    Token token;

    AstSelectorExpr() {
        kind = Ast_SelectorExpr;
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
    Token open;
    Token close;

    AstCallExpr() {
        kind = Ast_CallExpr;
    }
};

struct IfExpr : Ast {
    Ast *condition;
    Ast *then_expr = nullptr;
    IfExpr *else_if = nullptr;
    Token token;
    bool is_ifcase = false;

    IfExpr() {
        kind = Ast_IfExpr;
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
    Array<AstParam*> params;
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
    AstName *name;
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
