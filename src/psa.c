#include "psa.h"
DEFINE_STACK_FUNCTIONS(PSA_Token)

char P_TABLE[10][10] = {
    //!	   */   +-  LOG   ??   i    (    )	  $
    {'-', '>', '>', '>', '>', '<', '<', '>', '>'}, // !
    {'<', '>', '>', '>', '>', '<', '<', '>', '>'}, // */
    {'<', '<', '>', '>', '>', '<', '<', '>', '>'}, // +-
    {'<', '<', '<', '>', '>', '<', '<', '>', '>'}, // LOG
    {'<', '<', '<', '<', '>', '<', '<', '>', '>'}, // ??
    {'>', '>', '>', '>', '>', '-', '-', '>', '>'}, // i
    {'<', '<', '<', '<', '<', '<', '<', '=', '-'}, // (
    {'>', '>', '>', '>', '>', '-', '-', '>', '>'}, // )
    {'<', '<', '<', '<', '<', '<', '<', '-', '-'}, // $

    // LOG = LOGICAL OPERATOR (==, !=, <, >, <=, >=)
};

unsigned int getSymbolValue(Token_type token)
{
    // symbols: !,* /,+ -,== != < > <= >=,??,i,(,),$

    switch (token)
    {
    case TOKEN_NOT:
        return 0;
    case TOKEN_MUL:
    case TOKEN_DIV:
        return 1;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 2;
    case TOKEN_EQ:
    case TOKEN_NEQ:
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQ:
    case TOKEN_MORE_EQ:
    case TOKEN_AND:
    case TOKEN_OR:
        return 3;
    case TOKEN_BINARY_OPERATOR:
        return 4;
    case TOKEN_IDENTIFICATOR:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_EXP:
    case TOKEN_STRING:
        return 5;
    case TOKEN_L_BRACKET:
        return 6;
    case TOKEN_R_BRACKET:
        return 7;
    case TOKEN_EOF:
        return 8;
    default: // error
        return 99;
    }
}

uint32_t handleToUInt32(PSA_Token *handle, unsigned int len)
{
    uint32_t result = 0;
    for (int i = 0; i < len; i++)
    {
        result = result << 8 | (char)(handle[i].type);
    }
    return result;
}

uint32_t reverseHandleToUInt32(PSA_Token *handle, unsigned int len)
{
    uint32_t result = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        result = result << 8 | (char)(handle[i].type);
    }
    return result;
}

uint32_t reverseHandleTypesToUInt32(Expression_type *types, unsigned int len)
{
    uint32_t result = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        result = result << 8 | (char)types[i];
    }
    return result;
}

Expression_type getTypeFromToken(Token_type token)
{
    switch (token)
    {
    case TOKEN_INT:
        return (Expression_type)TYPE_INT;
    case TOKEN_DOUBLE:
    case TOKEN_EXP:
    case TOKEN_IDENTIFICATOR: // TODO: delete and for id use symbol table
        return (Expression_type)TYPE_DOUBLE;

    case TOKEN_STRING:
        return (Expression_type)TYPE_STRING;
    default:
        return (Expression_type)TYPE_INVALID;
    }
}

/**
 * @brief Returns the type of the expression based on the types of the operands.

    Supports: (int, int), (int, double), (double, int), (double, double)
 *
 * @param l_operand
 * @param r_operand
 * @return Expression_type
 */
Expression_type getTypeCombination(PSA_Token l_operand, PSA_Token r_operand)
{
    switch (((char)l_operand.expr_type << 8) | r_operand.expr_type)
    {
    case ((char)TYPE_INT << 8) | TYPE_INT:
        return TYPE_INT;
    case ((char)TYPE_INT << 8) | TYPE_DOUBLE:
        if (l_operand.type != TOKEN_IDENTIFICATOR)
        {
            DEBUG_CODE(printf("implicite Int2Double for left operand '%s'\n", l_operand.token_value););
            return TYPE_DOUBLE;
        }
        else
        {
            return TYPE_INVALID;
        }
    case ((char)TYPE_DOUBLE << 8) | TYPE_INT:
        if (r_operand.type != TOKEN_IDENTIFICATOR)
        {
            DEBUG_CODE(printf("impicite Int2Double for right operand '%s'\n", r_operand.token_value););
            return TYPE_DOUBLE;
        }
        else
        {
            return TYPE_INVALID;
        }
    case ((char)TYPE_DOUBLE << 8) | TYPE_DOUBLE:
        return TYPE_DOUBLE;
    default:
        return TYPE_INVALID;
    }
}

/**
 * @brief Returns the type of the expression based on the types of the operands and the operation.
 *
 * @param l_operand
 * @param operation
 * @param r_operand
 * @return Expression_type
 */
PSA_Token getHandleType(PSA_Token l_operand, Token_type operation, PSA_Token r_operand)
{
    if (l_operand.expr_type == TYPE_INVALID || r_operand.expr_type == TYPE_INVALID)
    {
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = TYPE_INVALID,
            .canBeNil = false,
        };
    }

    switch (operation)
    {
    // for: +, -, *, /
    case TOKEN_PLUS:
        // can be (string, string), ...
        if (l_operand.expr_type == TYPE_STRING && r_operand.expr_type == TYPE_STRING && !(l_operand.canBeNil || r_operand.canBeNil))
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = TYPE_STRING,
                .canBeNil = false,
            };
        }

    case TOKEN_MINUS:
    case TOKEN_MUL:
    case TOKEN_DIV:

        if (l_operand.canBeNil || r_operand.canBeNil)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = TYPE_INVALID,
                .canBeNil = false,
            };
        }

        // can be (int, int), (int, double), (double, int), (double, double)
        Expression_type type = getTypeCombination(l_operand, r_operand);

        if (type != TYPE_INVALID)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = type,
                .canBeNil = false,
            };
        }

        break;

    // for: ==, !=
    case TOKEN_EQ:
    case TOKEN_NEQ:

        // can be (int, int), (int, double), (double, int), (double, double), (string, string), (bool, bool)
        if (l_operand.expr_type == r_operand.expr_type || getTypeCombination(l_operand, r_operand) != TYPE_INVALID)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = TYPE_BOOL,
                .canBeNil = false,
            };
        }
        break;
    // for: <, >, <=, >=
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQ:
    case TOKEN_MORE_EQ:
        // can be (int, int), (int, double), (double, int), (double, double), (string, string)
        if (getTypeCombination(l_operand, r_operand) != TYPE_INVALID || l_operand.expr_type == TYPE_STRING && r_operand.expr_type == TYPE_STRING)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = TYPE_BOOL,
                .canBeNil = false,
            };
        }
        break;
    // for: ??, &&, ||
    case TOKEN_AND:
    case TOKEN_OR:
        if (l_operand.expr_type == TYPE_BOOL && r_operand.expr_type == TYPE_BOOL)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = TYPE_BOOL,
                .canBeNil = false,
            };
        }
        break;
    // for: !
    case TOKEN_BINARY_OPERATOR:
        if (l_operand.canBeNil && !r_operand.canBeNil)
        {
            return (PSA_Token){
                .type = (Token_type)TOKEN_EXPRSN,
                .token_value = "E",
                .expr_type = getTypeCombination(l_operand, r_operand),
                .canBeNil = false,
            };
        }
        break;
    default:
        break;
    }

    return (PSA_Token){
        .type = (Token_type)TOKEN_EXPRSN,
        .token_value = "E",
        .expr_type = TYPE_INVALID,
        .canBeNil = false,
    };
}

PSA_Token getRule(PSA_Token *handle, unsigned int len)
{
    uint32_t handle_val = handleToUInt32(handle, len);
    /*
        E -> i
        E -> (E)
        E -> !E
        E -> +E
        E -> -E
        E -> E*E
        E -> E/E
        E -> E+E
        E -> E-E
        E -> E==E
        E -> E!=E
        E -> E<E
        E -> E>E
        E -> E<=E
        E -> E>=E
        E -> E??E
    */
    switch (handle_val)
    {
    case RULE_1a:
    case RULE_1b:
    case RULE_1c:
    case RULE_1d:
    case RULE_1e:
        DEBUG_CODE(printf_cyan("rule: E -> i\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = getTypeFromToken(handle[0].type),
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_2:
        DEBUG_CODE(printf_cyan("rule: E -> (E)\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_3:
        DEBUG_CODE(printf_cyan("rule: E -> !E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_4:
        DEBUG_CODE(printf_cyan("rule: E -> +E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_5:
        DEBUG_CODE(printf_cyan("rule: E -> -E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_6:
        DEBUG_CODE(printf_cyan("rule: E -> E*E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_7:
        DEBUG_CODE(printf_cyan("rule: E -> E/E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_8:
        DEBUG_CODE(printf_cyan("rule: E -> E+E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_9:
        DEBUG_CODE(printf_cyan("rule: E -> E-E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_10:
        DEBUG_CODE(printf_cyan("rule: E -> E==E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_11:
        DEBUG_CODE(printf_cyan("rule: E -> E!=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_12:
        DEBUG_CODE(printf_cyan("rule: E -> E<E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_13:
        DEBUG_CODE(printf_cyan("rule: E -> E>E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_14:
        DEBUG_CODE(printf_cyan("rule: E -> E<=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_15:
        DEBUG_CODE(printf_cyan("rule: E -> E>=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_16:
        DEBUG_CODE(printf_cyan("rule: E -> E&&E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_17:
        DEBUG_CODE(printf_cyan("rule: E -> E||E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_18:
        DEBUG_CODE(printf_cyan("rule: E -> E??E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    default:
        DEBUG_CODE(printf_red("rule: EOF\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EOF,
            .token_value = "$",
            .expr_type = TYPE_INVALID,
            .canBeNil = false,
        };
    }

    return (PSA_Token){
        .type = (Token_type)TOKEN_EOF,
        .token_value = "$",
    };
}

bool isTokenOperand(Token_type token)
{
    switch (token)
    {
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_EXP:
    case TOKEN_STRING:
    case TOKEN_EXPRSN:
    case TOKEN_IDENTIFICATOR:
        return true;
    default:
        return false;
    }
}

bool isTokenBinaryOperator(Token_type token)
{
    switch (token)
    {
    case TOKEN_MUL:
    case TOKEN_DIV:
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_EQ:
    case TOKEN_NEQ:
    case TOKEN_LESS:
    case TOKEN_MORE:
    case TOKEN_LESS_EQ:
    case TOKEN_MORE_EQ:
    case TOKEN_AND:
    case TOKEN_OR:
    case TOKEN_BINARY_OPERATOR:
        return true;
    default:
        return false;
    }
}

bool canTokenBeStartOfExpression(Token_type token)
{
    switch (token)
    {
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_EXP:
    case TOKEN_STRING:
    case TOKEN_NOT:
    case TOKEN_L_BRACKET:
    case TOKEN_IDENTIFICATOR:
        return true;
    default:
        return false;
    }
}

bool canTokenBeEndOfExpression(Token_type token)
{
    switch (token)
    {
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_EXP:
    case TOKEN_STRING:
    case TOKEN_R_BRACKET:
    case TOKEN_IDENTIFICATOR:
        return true;
    default:
        return false;
    }
}

PSA_Token readNextToken(PSA_Token_stack *s, char *next_token_error)
{
    int ch = getchar();
    ungetc(ch, stdin);
    if (ch == EOF)
    {
        return (PSA_Token){
            .type = (Token_type)TOKEN_EOF,
            .token_value = "$",
        };
    }

    Token *tkn = malloc(sizeof(Token));
    generate_token(tkn, "\0");
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
    *next_token_error += isTokenOperand(a.type) && !isTokenBinaryOperator(b.type) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // detect expression end by an illegal token for expression being read
    *next_token_error += (getSymbolValue(b.type) == 99) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

    // detect empty expression
    *next_token_error += (a.type == (Token_type)TOKEN_EOF && !canTokenBeStartOfExpression(b.type)) ? 1 : 0;
    *next_token_error = *next_token_error << 1;

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
    // printf("%s:%d, ", ((PSA_Token *)top->data)->token_value, ((PSA_Token *)top->data)->expr_type);
    // printf("{'%s', %d} ", ((PSA_Token *)top->data)->token_value, ((PSA_Token *)top->data)->type);
}

// recursively prints the stack
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

psa_return_type parse_expression_base(bool is_param)
{
    PSA_Token_stack *s = PSA_Token_stack_init();
    PSA_Token_stack_push(s, (PSA_Token){
                                .type = (Token_type)TOKEN_EOF,
                                .token_value = "$"});

    /*
        a - token on the top of the stack
        b - token on the input
        s - stack
    */

    PSA_Token a = PSA_Token_stack_top(s);

    char next_token_error = 0;
    PSA_Token b = readNextToken(s, &next_token_error);

    while (!(a.type == (Token_type)TOKEN_EXPRSN && s->size == 2 && b.type == (Token_type)TOKEN_EOF))
    {
        // if the stack top is of type (Token_type)TOKEN_EXPRSN, then we need to use the second top of the stack to determine the rule
        if (a.type == (Token_type)TOKEN_EXPRSN)
        {
            a = s->top->next->data;
        }

        DEBUG_CODE(printf("Chyba: %d\n", next_token_error););

        // // DETECT EMPTY EXPRESSION
        // if (a.type == (Token_type)TOKEN_EOF && !canTokenBeStartOfExpression(b.type))
        // {
        //     // empty expression followed by EOL is valid
        //     if (b.preceded_by_nl)
        //     {
        //         return (psa_return_type){
        //             .end_token = TOKEN_EXPRSN,
        //             .is_ok = true,
        //             .canBeNil = false,
        //             .type = TYPE_EMPTY,
        //         };
        //     }
        //     else
        //     { // empty expression followed by something else is invalid
        //         printf_red("❌ | Error: invalid token! Token '%s' cannot be a part of an expression, add a newline as a separator. \n", b.token_value);

        //         return (psa_return_type){
        //             .end_token = TOKEN_EOF,
        //             .is_ok = false,
        //             .canBeNil = false,
        //             .type = TYPE_INVALID,
        //         };
        //     }
        // }

        DEBUG_CODE(printf("na stacku: ");
                   printStack(s->top);
                   printf_yellow("na vstupu: {'%s', %d}\n", b.token_value, b.type);
                   printf_magenta("P_TABLE[{%d, '%s'}][{%d, '%s'}] = %c\n", getSymbolValue(a.type), a.token_value, (b.type), b.token_value, P_TABLE[getSymbolValue(a.type)][getSymbolValue(b.type)]););

        const unsigned int a_val = getSymbolValue(a.type);
        const unsigned int b_val = getSymbolValue(b.type);

        if (a_val == 99 || b_val == 99)
        {
            printf_red("❌ | Error: invalid token! Unexpected token '%s'. \n", b.token_value);

            return (psa_return_type){
                .end_token = TOKEN_EOF,
                .is_ok = false,
                .canBeNil = false,
                .type = TYPE_INVALID,
            };
        }

        // TODO: dodelat funkce
        if (a.type == (Token_type)TOKEN_IDENTIFICATOR && b.type == (Token_type)TOKEN_L_BRACKET)
        {
            printf_magenta("Je to funkce!\n");
        }

        switch (P_TABLE[a_val][b_val])
        {
        case '=':
            PSA_Token_stack_push(s, b);
            b = readNextToken(s, &next_token_error);
            break;
        case '<':

            if (PSA_Token_stack_top(s).type == (Token_type)TOKEN_EXPRSN)
            {
                PSA_Token tmp = PSA_Token_stack_pop(s);
                PSA_Token_stack_push(s, (PSA_Token){
                                            .type = (Token_type)TOKEN_SHIFT,
                                            .token_value = "<"});
                PSA_Token_stack_push(s, tmp);
                PSA_Token_stack_push(s, b);
                b = readNextToken(s, &next_token_error);
            }
            else
            {
                PSA_Token_stack_push(s, (PSA_Token){
                                            .type = (Token_type)TOKEN_SHIFT,
                                            .token_value = "<"});
                PSA_Token_stack_push(s, b);
                b = readNextToken(s, &next_token_error);
            }
            break;
        case '>':
        { // from the top of the stack, pop all tokens until the first < is found
            // put all the popped tokens->type into an array
            // getTheRule of the array
            // if the rule is not EOF, push the rule into the stack
            // else, return error
            PSA_Token *handle = malloc(sizeof(PSA_Token) * s->size);

            int i = 0;
            while (PSA_Token_stack_top(s).type != TOKEN_SHIFT)
            {
                handle[i] = ((PSA_Token)PSA_Token_stack_pop(s));
                i++;
            }
            (void)PSA_Token_stack_pop(s); // pop the <

            // reverse the array
            for (int j = 0; j < i / 2; j++)
            {
                PSA_Token tmp = handle[j];
                handle[j] = handle[i - j - 1];
                handle[i - j - 1] = tmp;
            }

            PSA_Token rule = getRule(handle, i);

            if (rule.type != TOKEN_EOF)
            {
                PSA_Token_stack_push(s, rule);
            }
            else
            {
                printf_red("❌ | Error: invalid expression! Unexpected token '%s' in expression. \n", b.token_value);

                return (psa_return_type){
                    .end_token = TOKEN_EOF,
                    .is_ok = false,
                    .canBeNil = false,
                    .type = TYPE_INVALID,
                };
            }

            break;
        }
        case '-':
        default:
            printf_red("❌ | Error: invalid combination of operands! '%s' and '%s' cannot be together, because it wasn't meant to be. \n", a.token_value, b.token_value);

            return (psa_return_type){
                .end_token = TOKEN_EOF,
                .is_ok = false,
                .canBeNil = false,
                .type = TYPE_INVALID,
            };
        }

        printStack(s->top);
        DEBUG_CODE(printf("\n-----------\n\n"););

        a = PSA_Token_stack_top(s);
    }
    printf("\n");
    printf_green("✅ | All good! \n");

    return (psa_return_type){
        .is_ok = a.expr_type != TYPE_INVALID,
        .type = a.expr_type,
        .end_token = a.type,
        .canBeNil = a.canBeNil,
    };
}

psa_return_type parse_expression()
{
    return parse_expression_base(false);
}

psa_return_type parse_expression_param()
{
    return parse_expression_base(true);
}
