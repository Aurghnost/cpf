#ifndef _BNF_H_INCLUDED
#define _BNF_H_INCLUDED

#include "parser/dfa.h"
#include "parser/cfg.h"
/**
 * Grammaire ABNF :
 * %token = RULE_NAME / TERMINAL_BINARY / TERMINAL_DECIMAL /TERMINAL_BINARY 
 * 
 * rulelist = 1*( rule / ENDLINE )
 * rule = RULE_NAME defined-as elements ENDLINE
 * defined-as = ASSIGNATION / INCR_ALTERN
 * elements = alternation
 * alternation = concatenation *( "/ " concatenation)
 * concatenation = repetition *( repetition)
 * repetition = [repeat] element
 * repeat = 1*DIGIT / (*DIGIT "*" *DIGIT)
 * element = RULE_NAME / group / option / TERMINAL_LITTERAL_STRING / num
 * num = TERMINAL_DECIMAL / TERMINAL_BINARY / TERMINAL_HEXA
 * group = GROUP_OPEN alternation GROUP_CLOSE 
 * option = OPTION_OPEN alternation OPTION_CLOSE
 */
typedef enum {
  NONE,
  RULE_NAME,
  ASSIGNATION,
  TERMINAL_LITTERAL_STRING,
  TERMINAL_BINARY,
  TERMINAL_DECIMAL,
  TERMINAL_HEXA,
  PERIOD,
  DASH,
  ALTERNATIVE,
  INCR_ALTERN,
  ENDLINE,
  GROUP_OPEN,
  GROUP_CLOSE,
  OPTION_OPEN,
  OPTION_CLOSE,
  COUNT,
  STAR,
  _SIZEOF_BNF_SYMBOL
} bnf_symbol_t;

typedef enum {
  DEFAULT,
  SYMBOL,
  COMMENT,
  WHITESPACE,
  CHARACTER0,
  CHARACTER1,
  CHARACTER2,
  CHARACTER3,
  ALTERN,
  VRANGE,
  BRANGE,
  BRANGE_DOT,
  BRANGE_DASH,
  DRANGE,
  DRANGE_DOT,
  DRANGE_DASH,
  XRANGE,
  XRANGE_DOT,
  XRANGE_DASH,
  DVALUE,
  _SIZEOF_BNF_STATE
} bnf_state_t;

dfa_t bnf_lexer();
cfg_t bnf_grammar();

#endif
