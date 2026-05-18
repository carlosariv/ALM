#pragma once

struct AstFile;
struct Scope;

struct Symbol {
    Atom *name;
    Ast *node;
    Scope *scope;
};

struct Scope {
    Scope *parent = nullptr;
    Array<Symbol*> symbol_table;
};

struct Resolver {
    Scope *scope = nullptr;
};


void resolve_file(Resolver *R, AstFile *file);
