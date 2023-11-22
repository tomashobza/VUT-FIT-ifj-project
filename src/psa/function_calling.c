#include "psa.h"

PSA_Token parseFunctionCall(PSA_Token_stack *main_s, PSA_Token id, symtable_stack *st_stack)
{
    bool is_ok = true;

    // check if the id of the function is in the symtable
    symtable_item *found_func = symtable_find_in_stack(id.token_value, st_stack);
    if (found_func == NULL)
    {
        is_ok = false;

        printf("Function %s not found!\n", id.token_value);
        // TODO: if not -> error (for now)
        return (PSA_Token){
            .type = TOKEN_EXPRSN,
            .token_value = "E",
            .canBeNil = false,
            .expr_type = TYPE_INVALID,
            .preceded_by_nl = false};
    }

    printf("Function '%s' found\n", id.token_value);

    // TODO: check if function should have parameters
    if (found_func->data.func_data->params_count == 0 || found_func->data.func_data->params == NULL)
    {
        // TODO: handle function without parameters
    }

    // read the next token (should be ( token)
    char next_token_error = 0;
    if (readNextToken(main_s, &next_token_error).type != TOKEN_L_BRACKET)
    {
        // if not -> error
        // TODO: throw error
        is_ok = false;
    }

    // parse the next n parameters using parse_expression_param

    int param_counter = 0;
    bool params_ok = true;
    psa_return_type parsed_param = {0};

    do
    {
        printf("The name is: %s\n", checkParamName(main_s, param_counter, found_func) ? "ok" : "not ok");

        psa_return_type parsed_param = parse_expression_param(st_stack);

        printf("param[%d] has type: ", param_counter);
        print_expression_type(parsed_param.type);
        printf("\n");

        params_ok = params_ok && parsed_param.is_ok && parsed_param.type == found_func->data.func_data->params[param_counter].type;

        param_counter++;
    } while (param_counter < found_func->data.func_data->params_count && parsed_param.type != TYPE_EMPTY);

    is_ok = is_ok && params_ok;

    // read the next token (should be ) token)
    if (readNextToken(main_s, &next_token_error).type != TOKEN_R_BRACKET)
    {
        // if not -> error
        // TODO: throw error
        is_ok = false;
    }

    if (is_ok)
    {
        return (PSA_Token){
            .type = TOKEN_FUNC_ID,
            .token_value = found_func->id,
            .canBeNil = false,
            .expr_type = found_func->data.func_data->return_type,
            .preceded_by_nl = id.preceded_by_nl};
    }

    return (PSA_Token){
        .type = TOKEN_EOF,
        .token_value = "E",
        .canBeNil = false,
        .expr_type = TYPE_INVALID,
        .preceded_by_nl = false};
}

bool checkParamName(PSA_Token_stack *main_s, unsigned int param_index, symtable_item *found_func)
{
    // read the first token (should be an identificator)
    char next_token_error = 0;
    PSA_Token id = readNextToken(main_s, &next_token_error);

    // read the second token (should be : token)
    PSA_Token colon = readNextToken(main_s, &next_token_error);

    bool has_name = id.type == TOKEN_IDENTIFICATOR && colon.type == TOKEN_DOUBLE_DOT;

    // if the tokens don't match the pattern of a parameter name, return them
    if (!has_name)
    {
        return_token(convertPSATokenToToken(colon));
        return_token(convertPSATokenToToken(id));
    }

    bool should_have_name = param_index < (unsigned int)found_func->data.func_data->params_count && !strcmp(found_func->data.func_data->params[param_index].name, "_");

    bool name_is_ok = false;

    /*
        SEMANTIC RULES FOR PARAMETER NAME:
        1. should_have_name = true, has_name = true -> ok
        2. should_have_name = true, has_name = false -> error
        3. should_have_name = false, has_name = true -> error
        4. should_have_name = false, has_name = false -> ok
    */

    // TODO: handle all 4 error states
    if (has_name != should_have_name)
    {
        // 2. should have name, does not have name -> error
        // or
        // 3. should not have name, does have name -> error
        name_is_ok = false;
    }
    else
    {
        if (!should_have_name) // 4. should not have name, does not have name -> ok
        {
            name_is_ok = true;
        }
        else // 1. should have name, does have name -> ok
        {
            // check if the parameter name is correct according to the symtable
            if (!strcmp(found_func->data.func_data->params[param_index].id, id.token_value))
            {
                // the name is correct
                name_is_ok = true;
            }
        }
    }

    return name_is_ok;
}