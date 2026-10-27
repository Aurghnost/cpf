#ifndef _AST_H_INCLUDED
#define _AST_H_INCLUDED

#include "parser/cfg.h"
#include "parser/lexem.h"

typedef struct ast_node_s ast_node_t;
struct ast_node_s {
  ast_node_t * * children; //! Array of children nodes
  unsigned length; //! Number of children
  lexem_t label; //! Node label
};

typedef ast_node_t *ast_t;

typedef void (*symbol_printer_t)(FILE*, const lexem_t);

/**
 * @return An abstract syntax tree representing src according to g if g is LL(1), NULL otherwise.
 */
ast_t ast_from_ll1(const cfg_t g, table_ll1_t table, lexem_list_t src, symbol_printer_t sp);

/**
 * @return An abstract syntax tree representing src according to g, NULL if a parsing error occurs.
 */
ast_t ast_from_lr_table(const cfg_t g, lr_table_t table, lexem_list_t src, symbol_printer_t sp);

void ast_destroy(ast_t tree);

#endif
