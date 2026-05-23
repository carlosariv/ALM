#pragma once

struct AstFile;
struct Scope;
struct Type;
struct Parser;

enum ResolveState {
    ResolveState_NotStarted,
    ResolveState_Begun,
    ResolveState_Completed,
};

enum DeclKind {
    Decl_Nil,
    Decl_Constant,
    Decl_Var,
    Decl_Type,
    Decl_ProcGroup,
    Decl_Proc,
};

struct Decl {
    DeclKind kind = Decl_Nil;
    ResolveState resolve_state = ResolveState_NotStarted;
    Atom *name = nullptr;
    Scope *scope = nullptr;

    ValueDecl *vd = nullptr;
    Type *type = nullptr;

    // ProcGroup
    Array<Decl*> procedures;
    ProcLit *proc_lit = nullptr;

    ComptimeValue ct_value;
};

struct Scope {
    Scope *parent = nullptr;
    Ast *node = nullptr;
    Array<Decl*> decl_table;
};

struct Resolver {
    Scope *global_scope = nullptr;
    Scope *scope = nullptr;

    Array<AstFile*> files;

    Ast *control_target = nullptr;
};


void resolve_program(Resolver *R, Parser *P);
void resolve_file(Resolver *R, AstFile *file);
void resolve_expr(Resolver *R, Ast *expr);

