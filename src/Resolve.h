#pragma once

struct AstFile;
struct Scope;
struct Type;
struct Parser;

enum ResolveState {
    ResolveState_UnInit,
    ResolveState_InProgress,
    ResolveState_Completed,
};

enum DeclKind {
    Decl_Nil,
    Decl_Var,
    Decl_Type,
    Decl_ProcGroup,
    Decl_Proc,
    Decl_Constant,
};

struct Decl {
    DeclKind kind = Decl_Nil;
    ResolveState resolve_state = ResolveState_UnInit;
    Atom *name = nullptr;
    Scope *scope = nullptr;

    Ast *node = nullptr;
    Ast *init_expr = nullptr;
    Ast *type_defn = nullptr;
    ProcLit *proc_lit = nullptr;

    Type *type = nullptr;
    ComptimeValue ct_value;

    Array<Decl*> procedures; 
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

