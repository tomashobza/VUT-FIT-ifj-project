#include "psa.h"

PSA_Token readNextToken(PSA_Token_stack *s, char *next_token_error, int *num_of_brackets)
{
    Token *tkn = malloc(sizeof(Token));
    *tkn = (Token){
        .type = (Token_type)TOKEN_EOF,
        .token_value = "$",
        .preceded_by_nl = true,
    };

    main_scanner(tkn);

    PSA_Token b = {
        .type = tkn->type,
        .token_value = tkn->token_value,
        .expr_type = getTypeFromToken(tkn->type),
        .canBeNil = false,
        .preceded_by_nl = tkn->preceded_by_nl,
    };
    free(tkn);

    PSA_Token a = PSA_Token_stack_top(s);

    *next_token_error = 0;

    // detect expression end by a missing operator between operands
    *next_token_error += isTokenOperand(a.type) && !isTokenBinaryOperator(b.type) && !isTokenBracket(b.type) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // detect expression end by an illegal token for expression being read
    *next_token_error += (getSymbolValue(b.type) >= 99) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // TODO: remove
    // detect empty expression
    // *next_token_error += (a.type == (Token_type)TOKEN_EOF && !canTokenBeStartOfExpression(b.type)) ? 1 : 0;
    // *next_token_error = *next_token_error << 1;

    DEBUG_PSA_CODE(printf_cyan("'%s' next_token_error: %d\n", b.token_value, *next_token_error););

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
