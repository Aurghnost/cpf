#ifndef _RENDERER_H_INCLUDED
#define _RENDERER_H_INCLUDED

#include "help_page_renderer/grammar.h"
#include "parser/cfg.h"
#include "parser/ast.h"

void renderer_render(int fdout, const char * filename);

#endif // !_RENDERER_H_INCLUDED
