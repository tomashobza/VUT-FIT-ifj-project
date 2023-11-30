/**
 * @file io.c
 * @author Tomáš Hobza (xhobza03@vutbr.cz)
 * @brief Functions for input (reading the next token of the expression) and output (printing, mainly) needs for the PSA.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "psa.h"

PSA_Token readNextToken(PSA_Token_stack *s, char *next_token_error, int *num_of_brackets)
{
    Token *tkn = malloc(sizeof(Token));
    *tkn = (Token){
        .type = (Token_type)TOKEN_EOF,
        .token_value = "$",
        .preceded_by_nl = true,
    };

    // TODO: hodit (si lano/chybu)
    Error_code scanner_returned = (Error_code)main_scanner(tkn);
    if (scanner_returned != NO_ERR)
    {
        throw_error(scanner_returned, "Scanner error.");
        printf_red("\nSCANNER VRATIL: ");
        printError((Error){
            .code = scanner_returned,
            .line_num = line_num,
            .message = "Scanner error."});
        printf("\n\n");
    }

    if (tkn->type == TOKEN_EOF)
    {
        return_token(*tkn);
    }

    PSA_Token b = {
        .type = tkn->type,
        .token_value = tkn->token_value,
        .expr_type = getTypeFromToken(tkn->type),
        .preceded_by_nl = tkn->type == TOKEN_EOF ? true : tkn->preceded_by_nl,
    };
    free(tkn);

    PSA_Token a = (PSA_Token){
        .type = TOKEN_EOF,
        .token_value = "$",
        .expr_type = TYPE_INVALID,
        .preceded_by_nl = true,
    };
    if (s != NULL && !PSA_Token_stack_empty(s))
    {
        a = PSA_Token_stack_top(s);
    }

    *next_token_error = 0;

    // detect expression end by a missing operator between operands
    *next_token_error += isTokenOperand(a.type) && !isTokenBinaryOperator(b.type) && !isTokenBracket(b.type) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // detect expression end by an illegal token for expression being read
    *next_token_error += (getSymbolValue(b.type) >= 99) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // update the bracket counter
    if (num_of_brackets != NULL)
    {
        switch (b.type)
        {
        case TOKEN_L_BRACKET:
            (*num_of_brackets)++;
            break;
        case TOKEN_R_BRACKET:
            (*num_of_brackets)--;
            break;
        default:
            break;
        }
    }

    return b;
}

void printStackRec(PSA_Token_node *top)
{
    if (top == NULL)
    {
        return;
    }
    printStackRec(top->next);
    printf("%s", top->data.token_value);
    // TODO: cleanup
    // printf("%s:%d, ", ((PSA_Token *)top->data)->token_value, ((PSA_Token *)top->data)->expr_type);
    // printf("{'%s', %d} ", ((PSA_Token *)top->data)->token_value, ((PSA_Token *)top->data)->type);
}

void printStack(PSA_Token_node *top)
{
    if (top == NULL)
    {
        printf("Stack empty!\n");
        return;
    }
    printStackRec(top);
    printf("\n");
}

void printTokenArray(PSA_Token *handle, unsigned int len)
{
    printf("handle: '");
    for (unsigned int i = 0; i < len; i++)
    {
        printf("%s", handle[i].token_value);
    }
    printf("'\n");
}
