/**
 * @file main.c
 * @brief main function of the program
 * @author Tomáš Hobza
 *
 * Project: IFJ compiler
 */

#include <stdio.h>
#include <stdlib.h>
#include "symtable.h"

symtable_stack *sym_st;

#include "psa.h"
#include "error.h"
#include "parser.h"
#include "utils.h"

int main(void)
{
    scanner_init();

    sym_st = symtable_stack_init();

    add_builtin_functions();

    parser_main();

    // print errors and return code
    return print_errors();
}
