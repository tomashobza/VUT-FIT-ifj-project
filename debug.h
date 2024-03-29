/**
 * @file debug.h
 * @author Jakub Všetečka <xvsete00@vutbr.cz>
 * @brief Debugging macros.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 * Project: IFJ compiler
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

//! DEBUG
#if DEBUG_PSA
#define DEBUG_PSA_CODE(code) \
    do                       \
    {                        \
        code                 \
    } while (0)
#else
#define DEBUG_PSA_CODE(code) \
    do                       \
    {                        \
    } while (0)
#endif

#if DEBUG_SYNTAX
#define DEBUG_SYNTAX_CODE(code) \
    do                          \
    {                           \
        code                    \
    } while (0)
#else
#define DEBUG_SYNTAX_CODE(code) \
    do                          \
    {                           \
    } while (0)
#endif

#if DEBUG_SEMANTIC
#define DEBUG_SEMANTIC_CODE(code) \
    do                            \
    {                             \
        code                      \
    } while (0)
#else
#define DEBUG_SEMANTIC_CODE(code) \
    do                            \
    {                             \
    } while (0)
#endif

#if DEBUG_LEXER
#define DEBUG_LEXER_CODE(code) \
    do                         \
    {                          \
        code                   \
    } while (0)
#else
#define DEBUG_LEXER_CODE(code) \
    do                         \
    {                          \
    } while (0)
#endif

#endif // DEBUG_H