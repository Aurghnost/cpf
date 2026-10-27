#include "unittest.h"

#include "parser/lexem.h"
#include "parser/dfa.h"
#include "parser/abnf.h"

void lexem_list_print(lexem_list_t *l) {
  for (lexem_node_t *nd = l->head; nd != NULL; nd = nd->next) {
    fprintf(stderr, "(%u, %s)", nd->lex.symbol, nd->lex.repr.buf);
  }
  fprintf(stderr, "\n");
}

unittest_init();
unittest(
  unittest_test("Liste de lexèmes", {
    
  }, UNITTEST_TODO)
  unittest_test("Automate BNF", {
  dfa_t a = bnf_lexer();

  lexem_list_t l = dfa_execute(a, "rule = b / c\nr =/ d / \"salut\""); 
  // fprintf(stderr, "%u\n", l.length);
  // lexem_list_print(&l);

  lexem_list_destroy(&l);
  dfa_destroy(&a);
	}, UNITTEST_DONE)
)
