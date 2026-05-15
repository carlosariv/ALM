#pragma once

#include <vector>

#include "BaseTypes.h"
#include "Array.h"
#include "Token.h"

struct Ast;
struct Type;

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

    Ast_Case,
    Ast_Do,
    Ast_While,
    Ast_For,

    Ast_Break,
    Ast_Continue,
    Ast_Fallthrough,
    Ast_Return,

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
    Ast_StarExpr,

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
    Token token;

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
    bool is_ifcase = false;
    Ast *trailing = nullptr;
    Token open;
    Token close;

    AstBlockExpr() {
        kind = Ast_BlockExpr;
    }
};

struct AstStarExpr : Ast {
    Ast *elem;
    Token token;

    AstStarExpr() {
        kind = Ast_StarExpr;
    }
};

struct AstEmptyStmt : Ast {
    Token token;
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

struct AstCase : Ast {
    Ast *expr;
    bool is_default;
    Array<Ast*> statements;
    AstCase *prev_clause;
    AstCase *next_clause;
    Token token;

    AstCase() {
        kind = Ast_Case;
    }
};

struct AstWhile : Ast {
    Ast *condition;
    AstBlockExpr *block;
    Token token;

    AstWhile() {
        kind = Ast_While;
    }
};

struct AstDo : Ast {
    Ast *condition;
    AstBlockExpr *block;
    Token token;

    AstDo() {
        kind = Ast_Do;
    }
};

struct AstFor : Ast {
    Ast *condition;
    AstBlockExpr *block;
    Token token;

    AstFor() {
        kind = Ast_For;
    }
};

struct AstContinue : Ast {
    Token token;

    AstContinue() {
        kind = Ast_Continue;
    }
};

struct AstBreak : Ast {
    Token token;

    AstBreak() {
        kind = Ast_Break;
    }
};

struct AstFallthrough : Ast {
    Token token;

    AstFallthrough() {
        kind = Ast_Fallthrough;
    }
};


struct AstReturn : Ast {
    Ast *expr;
    Token token;

    AstReturn() {
        kind = Ast_Return;
    }
};

struct AstProcType : Ast {
    Array<Ast*> params;
    Ast *return_type;

    AstProcType() {
        kind = Ast_ProcType;
    }
};

struct AstArrayType : Ast {
    Ast *elem;
    Ast *size;
    bool dynamic;
    Token open;
    Token close;

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

struct AstProcLit : Ast {
    AstProcType *proc_type;
    Ast *body;

    AstProcLit() {
        kind = Ast_ProcLit;
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

