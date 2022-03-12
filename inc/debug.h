#ifndef H_DEBUG
#define H_DEBUG
#endif

#ifdef DEBUG
#define DEBUG_EXPR(expr) expr
#else
#define DEBUG_EXPR(expr)
#endif  