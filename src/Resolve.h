#pragma once

struct AstFile;
struct Scope;
struct Type;

enum SymbolState {
    SymbolState_Unresolved = (1<<0),
    SymbolState_Resolving  = (1<<1),
    SymbolState_Resolved   = (1<<2),
};

enum SymbolKind {
    Symbol_Var,
    Symbol_Type,
};

struct Symbol {
    SymbolKind kind;
    SymbolState state = SymbolState_Unresolved;
    Atom *name;
    Scope *scope;
    ValueDecl *vd = nullptr;
    Type *type = nullptr;
};

struct Scope {
    Scope *parent = nullptr;
    Array<Symbol*> symbol_table;
};

struct Resolver {
    Scope *global_scope = nullptr;
    Scope *scope = nullptr;
};


void resolve_file(Resolver *R, AstFile *file);
