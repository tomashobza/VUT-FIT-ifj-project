/**
 * @file semantic.c
 * @author Jakub Všetečka <xvsete00@vutbr.cz>
 * @brief Semantic analysis.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 * Project: IFJ compiler
 *
 */

#include "semantic.h"

void sem_start(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->funcItem = NULL;
}

void sem_let(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->varItem = init_symtable_item(false);
    items->varItem->data.var_data->is_const = true;
}

void sem_var(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->varItem = init_symtable_item(false);
    items->varItem->data.var_data->is_const = false;
}

void sem_var_id(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    // var id can't be _
    if (strcmp(token->token_value, "_") == 0)
    {
        throw_error(SYNTACTIC_ERR, token->line_num, "Variable name can't be '_'");
    }

    // check if var is already defined
    symtable_item *var_id_item = symtable_find(token->token_value, symtable_stack_top(sym_st), false);
    if (var_id_item != NULL && var_id_item->data.var_data->is_param == false)
    {
        throw_error(FUNCTIONS_ERR, token->line_num, "Variable %s is already defined!", token->token_value)
    }

    // check if var is param
    else if (var_id_item != NULL && var_id_item->data.var_data->is_param == true)
    {
        items->varItem->data.var_data->is_param = true;
    }

    items->varItem->id = token->token_value;
}

void sem_var_type(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->varItem->data.var_data->type = get_expression_type(token);
}

void sem_var_exp(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->varItem->data.var_data->is_initialized = true;

    DEBUG_SEMANTIC_CODE(
        (symtable_stack_top(sym_st)););

    // parse expression
    psa_return_type return_type = parse_expression();

    DEBUG_SEMANTIC_CODE(print_expression_type(return_type.type););

    // check if expression is valid
    if (return_type.is_ok == false)
    {
        if (items->varItem->data.var_data->type == TYPE_EMPTY)
        {
            throw_error(TYPE_ERR, token->line_num, "Unrecognizable type of variable: %s\n", items->varItem->id);
        }
        else
        {
            throw_error(COMPATIBILITY_ERR, token->line_num, "Unrecognizable type of variable: %s\n", items->varItem->id);
        }
    }
    else if (return_type.type == TYPE_NIL || return_type.type == TYPE_EMPTY)
    {
        // check if var can be nil
        if (items->varItem->data.var_data->type == TYPE_EMPTY)
        {
            throw_error(TYPE_ERR, token->line_num, "Couldn't decide the type of %s from type NIL!\n", items->varItem->id);
        }
        else if (items->varItem->data.var_data->type != TYPE_INT_NIL && items->varItem->data.var_data->type != TYPE_STRING_NIL && items->varItem->data.var_data->type != TYPE_DOUBLE_NIL && items->varItem->data.var_data->type != TYPE_BOOL_NIL)
        {
            throw_error(COMPATIBILITY_ERR, token->line_num, "Expression type: %d and type: %d of variable: %s do not match!\n", return_type.type, items->varItem->data.var_data->type, items->varItem->id);
        }
    }
    // if var's type is empty, set it to expression type
    else if (items->varItem->data.var_data->type == TYPE_EMPTY)
    {
        items->varItem->data.var_data->type = return_type.type;
    }
    // check if expression type and var type match, conversion is possible
    else if (!(check_ret_values(return_type.type, items->varItem->data.var_data->type) || isTypeConvertable(items->varItem->data.var_data->type, return_type.type, return_type.is_literal)))
    {
        throw_error(COMPATIBILITY_ERR, token->line_num, "Expression type: %d and type: %d of variable: %s do not match!\n", return_type.type, items->varItem->data.var_data->type, items->varItem->id);
    }

    DEBUG_SEMANTIC_CODE(
        (symtable_stack_top(sym_st)););

    // convert int to float
    if (!check_ret_values(return_type.type, items->varItem->data.var_data->type) && isTypeConvertable(items->varItem->data.var_data->type, return_type.type, return_type.is_literal))
    {
        generate_instruction(INT2FLOATS);
    }
}

void sem_var_add(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    // check if var is in uninitialized and can be nil
    if (items->varItem->data.var_data->is_initialized == false)
    {
        Expression_type type = items->varItem->data.var_data->type;
        if (type == TYPE_INT_NIL || type == TYPE_DOUBLE_NIL || type == TYPE_STRING_NIL || type == TYPE_BOOL_NIL)
        {
            items->varItem->data.var_data->is_initialized = true;
        }
    }

    if (items->varItem->data.var_data->is_param == false) // new symbol
    {
        DEBUG_SEMANTIC_CODE(printf(YELLOW "ADDING VAR: %s, type: %d, const: %d\n", items->varItem->id, items->varItem->data.var_data->type, items->varItem->data.var_data->is_const););
        symtable_add(items->varItem, symtable_stack_top(sym_st));
        DEBUG_SEMANTIC_CODE(symtable_print(symtable_stack_top(sym_st)););
    }
    else // is param
    {
        symtable_item *var_add_item = symtable_find_in_stack(items->varItem->id, sym_st, false);
        var_add_item->data.var_data->type = items->varItem->data.var_data->type;
        var_add_item->data.var_data->is_const = items->varItem->data.var_data->is_const;
        var_add_item->data.var_data->is_initialized = items->varItem->data.var_data->is_initialized;
        var_add_item->data.var_data->is_param = false; // is no longer param
    }
}

void sem_func_id(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->funcItem = init_symtable_item(true);
    symtable_item *func_id_item = symtable_find_in_stack(token->token_value, sym_st, true);

    // check if function is already defined
    if (func_id_item != NULL) // is in stack
    {
        if (func_id_item->type == FUNCTION)
        { // is function
            throw_error(FUNCTIONS_ERR, token->line_num, "Function %s is already defined!\n", token->token_value);
        }
    }

    items->funcItem->id = token->token_value;
}

void sem_p_name(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    add_param(items->funcItem->data.func_data);

    // sets param's name to token value
    items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].name = token->token_value;
}

void sem_p_id(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    if (strcmp(token->token_value, items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].name) == 0)
    {
        throw_error(SEMANTICS_ERR, token->line_num, "Parameter name: '%s' matches parameter id", token->token_value);
    }

    // parameters must be accesible as variables in function's body's scope
    items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].id = token->token_value;
    items->varItem = init_symtable_item(false);
    items->varItem->id = token->token_value;
}

void sem_p_type(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].type = get_expression_type(token);

    // check param
    if (items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].name == items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].id)
    {
        throw_error(FUNCTIONS_ERR, token->line_num, "Parameter name matches parameter id");
    }
    // check if param is already defined
    for (int i = 0; i < items->funcItem->data.func_data->params_count - 1; i++)
    {
        DEBUG_SEMANTIC_CODE(printf(CYAN "ADDED PARAM: %s, id: %s, type: %d\n", items->funcItem->data.func_data->params[i].name, items->funcItem->data.func_data->params[i].id, items->funcItem->data.func_data->params[i].type););
        if (strcmp(items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].id, items->funcItem->data.func_data->params[i].id) == 0)
        {
            throw_error(FUNCTIONS_ERR, token->line_num, "Parameter: '%s' in function: '%s' is already defined", items->funcItem->data.func_data->params[items->funcItem->data.func_data->params_count - 1].id, items->funcItem->id);
        }
    }
}

void sem_r_type(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    items->funcItem->data.func_data->return_type = get_expression_type(token);
}

void sem_func_header_done(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    // adds function to symtable
    DEBUG_SEMANTIC_CODE(printf(YELLOW "ADDING FUNC: %s, return type: %d\n", items->funcItem->id, items->funcItem->data.func_data->return_type););
    symtable_add(items->funcItem, symtable_stack_top(sym_st));
    DEBUG_SEMANTIC_CODE(symtable_print(symtable_stack_top(sym_st)););

    // push scope
    DEBUG_SEMANTIC_CODE(printf(RED "PUSH_SCOPE\n" RESET););
    symtable symtable = symtable_init();
    symtable_stack_push(sym_st, symtable);

    // add params as vars to new scope
    if (items->funcItem->data.func_data->params_count <= 0) // no params
    {
        return;
    }
    for (int i = 0; i < items->funcItem->data.func_data->params_count; i++)
    {
        if (strcmp(items->funcItem->data.func_data->params[i].id, "_") != 0)
        {
            items->varItem = init_symtable_item(false);
            items->varItem->id = items->funcItem->data.func_data->params[i].id;
            items->varItem->data.var_data->type = items->funcItem->data.func_data->params[i].type;
            items->varItem->data.var_data->is_const = true;
            items->varItem->data.var_data->is_initialized = true;
            items->varItem->data.var_data->is_param = true;
            symtable_add(items->varItem, symtable_stack_top(sym_st));
            DEBUG_SEMANTIC_CODE(printf(YELLOW "ADDING VAR TO FUNCTIONS: %s, type: %d, const: %d\n", items->varItem->id, items->varItem->data.var_data->type, items->varItem->data.var_data->is_const););
        }
    }
    DEBUG_SEMANTIC_CODE(symtable_print(symtable_stack_top(sym_st)););
}

void sem_push_scope(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    DEBUG_SEMANTIC_CODE(printf(RED "PUSH_SCOPE\n" RESET););
    symtable symtable = symtable_init();
    symtable_stack_push(sym_st, symtable);
}

void sem_pop_scope(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    DEBUG_SEMANTIC_CODE(printf(RED "POP_SCOPE\n"););
    symtable symt = symtable_stack_pop(sym_st);

    // return checking logic
    //
    // scope1 _
    //       | | _ scope11
    //       | || |
    //       | || |
    //       | ||_|             <-  S1.found_return = S1.found_return && S11.found_return
    //       | | _ scope12
    //       | || |
    //       | || |
    //       | ||_|             <-  S1.found_return = S1.found_return && S12.found_return
    //       | | _ scope12
    //       | || |
    //       | || |
    //       | ||_|             <-  S1.found_return = S1.found_return && S13.found_return
    //       | |
    //       |_|
    //
    // only looking at if we found return in every child scope is not enough to tell if the actual scope (scope1) needs to have return statement
    // we must check if every 'if' block ends with 'else' aswell
    // otherwise return statement must be present in scope1
    if (items->funcItem != NULL)
    {
        // process self
        symtable_stack_top(sym_st)->found_return = (symt->all_children_return && symt->found_else) || symt->found_return;

        // process lower scope
        symtable_stack_top(sym_st)->found_return &= symt->found_return; // bitwise AND assing
        symtable_stack_top(sym_st)->found_else = symt->found_else;
    }
}

void sem_func_if_start(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    symtable_stack_top(sym_st)->found_else = false;
}

void sem_r_exp(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    psa_return_type return_type2 = parse_expression();

    // check if return type matches function's return type
    if (((items->funcItem->data.func_data->return_type == TYPE_EMPTY) ^ (return_type2.type == TYPE_EMPTY))) // items->funcItem_type XOR exp_type
    {
        throw_error(RETURN_ERR, token->line_num, " ");
    }
    else if (return_type2.is_ok == false)
    {
        throw_error(PARAM_TYPE_ERR, token->line_num, "Unrecognizable type of return value in function: %s \n", items->funcItem->id);
    }
    else if (!(check_ret_values(return_type2.type, items->funcItem->data.func_data->return_type)))
    {
        throw_error(PARAM_TYPE_ERR, token->line_num, "Expression type: %d and return type: %d of function: %s do not match!\n", return_type2.type, items->funcItem->data.func_data->return_type, items->funcItem->id);
    }

    symtable_stack_top(sym_st)->found_return = true;
    DEBUG_SEMANTIC_CODE(print_expression_type(return_type2.type););
}

void sem_cond_exp(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    DEBUG_SEMANTIC_CODE(printf(CYAN "COND EXP: %s\n", token->token_value);
                        symtable_print(symtable_stack_top(sym_st)););

    psa_return_type return_type3 = parse_expression();

    DEBUG_SEMANTIC_CODE(printf(CYAN "COND EXP: %d\n", return_type3.type););

    if (return_type3.is_ok == false)
    {
        throw_error(COMPATIBILITY_ERR, token->line_num, "Invalid expression!\n");
    }
    else if (return_type3.type != TYPE_BOOL)
    {
        throw_error(COMPATIBILITY_ERR, token->line_num, "Expression type: %d and type: %d of variable: %s do not match!\n", return_type3.type, TYPE_BOOL, items->varItem->id);
    }

    DEBUG_SEMANTIC_CODE(print_expression_type(return_type3.type););
}

void sem_func_else(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    symtable_stack_top(sym_st)->found_else = true;
}

void sem_let_in_if(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    items->varItem = init_symtable_item(false);
    symtable_item *let_in_if_item = symtable_find_in_stack(token->token_value, sym_st, false);

    if (let_in_if_item == NULL || let_in_if_item->data.var_data->is_const == false)
    {
        throw_error(SEMANTICS_ERR, token->line_num, "Variable %s is not a defined const!\n", token->token_value);
    }

    switch (let_in_if_item->data.var_data->type)
    {
    case TYPE_BOOL_NIL:
        items->varItem->data.var_data->type = TYPE_BOOL;
        break;
    case TYPE_DOUBLE_NIL:
        items->varItem->data.var_data->type = TYPE_DOUBLE;
        break;
    case TYPE_INT_NIL:
        items->varItem->data.var_data->type = TYPE_INT;
        break;
    case TYPE_STRING_NIL:
        items->varItem->data.var_data->type = TYPE_STRING;
        break;
    default:
        break;
    }

    // 'if let id' id type shloudn't include nil in first block
    items->varItem->id = token->token_value;
    items->varItem->data.var_data->is_const = true;
    items->varItem->data.var_data->is_initialized = true;
    items->varItem->data.var_data->is_param = false;

    // push new scope
    DEBUG_SEMANTIC_CODE(printf(RED "PUSH_SCOPE\n" RESET););
    symtable symtable = symtable_init();
    symtable_stack_push(sym_st, symtable);

    // add var to new scope
    symtable_add(items->varItem, symtable_stack_top(sym_st));

    DEBUG_SEMANTIC_CODE(symtable_print(symtable_stack_top(sym_st)););
}

void sem_func_body_done(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    symtable_item *func_body_item = symtable_find_in_stack(items->funcItem->id, sym_st, true);
    if (symtable_stack_top(sym_st)->found_return == false && func_body_item->data.func_data->return_type != TYPE_EMPTY)
    {
        throw_error(PARAM_TYPE_ERR, token->line_num, "Function %s of type: %d does not have a return statement!\n", items->funcItem->id, func_body_item->data.func_data->return_type);
    }

    // reset atributtes for checking return logic
    symtable_stack_top(sym_st)->found_return = false;
    symtable_stack_top(sym_st)->found_else = false;
    symtable_stack_top(sym_st)->all_children_return = true;
}

void sem_load_identif(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{

    items->varItem = init_symtable_item(false);

    DEBUG_SEMANTIC_CODE(printf(CYAN);
                        symtable_print(symtable_stack_top(sym_st)););

    symtable_item *item = symtable_find_in_stack(token->token_value, sym_st, false);

    if (item == NULL)
    {
        throw_error(VARIABLES_ERR, token->line_num, "Variable %s is not defined!\n", token->token_value);
    }
    else if (item->data.var_data->is_const == true)
    {
        throw_error(SEMANTICS_ERR, token->line_num, "Variable %s is const!\n", token->token_value);
    }
    items->varItem->id = token->token_value;
}

void sem_identif_exp(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    DEBUG_SEMANTIC_CODE(printf(CYAN);
                        symtable_print(symtable_stack_top(sym_st)););

    psa_return_type return_type4 = parse_expression();

    DEBUG_SEMANTIC_CODE(print_expression_type(return_type4.type););

    if (return_type4.is_ok == false)
    {
        throw_error(COMPATIBILITY_ERR, token->line_num, "Unrecognizable type of variable: %s \n", items->varItem->id);
    }

    symtable_item *identif_exp_item = symtable_find_in_stack(items->varItem->id, sym_st, false);

    // check if expression type and var type match, conversion is possible
    if (!(check_ret_values(return_type4.type, identif_exp_item->data.var_data->type) || isTypeConvertable(identif_exp_item->data.var_data->type, return_type4.type, return_type4.is_literal)))
    {
        throw_error(COMPATIBILITY_ERR, token->line_num, "Expression type: %d and type: %d of variable: %s do not match!\n", return_type4.type, identif_exp_item->data.var_data->type, items->varItem->id);
    }

    // generate instructuions for conversion
    if (!check_ret_values(return_type4.type, identif_exp_item->data.var_data->type) && isTypeConvertable(identif_exp_item->data.var_data->type, return_type4.type, return_type4.is_literal))
    {
        generate_instruction(INT2FLOATS);
    }
}

void sem_func_call_psa(__attribute__((unused)) Token *token, __attribute__((unused)) sym_items *items)
{
    parse_expression();

    DEBUG_SEMANTIC_CODE(print_expression_type(return_type5.type););
}