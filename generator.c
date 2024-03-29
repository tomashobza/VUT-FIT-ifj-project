/**
 * @file generator.c
 * @author Tomáš Hobza (xhobza03@vutbr.cz)
 * @brief Generator of IFJcode20.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 * Project: IFJ compiler
 *
 */

#include "generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// GLOBAL COUNTERS

int if_counter = 0;
int while_counter = 0;
int tmp_counter = 0;
int label_counter = 0;
int_stack *else_label_st = NULL;

DEFINE_STACK_FUNCTIONS(int);

/// OPERAND FUNCTIONS

char *label(char *name)
{
    char *label = malloc(sizeof(char) * (strlen(name) + 1));
    sprintf(label, "%s", name);
    return label;
}

char *type(Expression_type type)
{
    char *type_str = malloc(sizeof(char) * 10);
    switch (type)
    {
    case TYPE_INT:
    case TYPE_INT_NIL:
        sprintf(type_str, "int");
        break;
    case TYPE_DOUBLE:
    case TYPE_DOUBLE_NIL:
        sprintf(type_str, "float");
        break;
    case TYPE_STRING:
    case TYPE_STRING_NIL:
        sprintf(type_str, "string");
        break;
    case TYPE_BOOL:
    case TYPE_BOOL_NIL:
        sprintf(type_str, "bool");
        break;
    case TYPE_NIL:
        sprintf(type_str, "nil");
        break;
    case TYPE_INVALID:
        sprintf(type_str, "invalid");
        break;
    default:
        sprintf(type_str, "unknown");
        break;
    }
    return type_str;
}

char *variable(char *id, int scope, bool has_suffix)
{
    char *var_name = malloc(sizeof(char) * (10 + strlen(id)));
    char *scope_prefix = scope == 0 ? "GF" : (scope < 0 ? "TF" : "LF"); // scope determies the frame prefix
    char *suffix = malloc(sizeof(char) * 10);
    sprintf(suffix, "%d", scope);

    sprintf(var_name, "%s@$%s%s", scope_prefix, id, has_suffix ? suffix : "");
    return var_name;
}

char *literal(Token token)
{
    return format_token(token);
}

char *symbol(Token symbol)
{
    return symb_resolve(symbol);
}

/// INSTRUCTION FUNCTIONS

void handle_0_operand_instructions(Instruction inst)
{
    char *instruction = instructionToString(inst);
    fprintf(out_code_file, "%s\n", instruction);
    free(instruction);
}

void handle_1_operand_instructions(Instruction inst, char *op1)
{
    char *instruction = instructionToString(inst);
    fprintf(out_code_file, "%s %s\n", instruction, op1);
    free(instruction);
}

void handle_2_operand_instructions(Instruction inst, char *op1, char *op2)
{
    char *instruction = instructionToString(inst);
    fprintf(out_code_file, "%s %s %s\n", instruction, op1, op2);
    free(instruction);
}

void handle_3_operand_instructions(Instruction inst, char *op1, char *op2, char *op3)
{
    char *instruction = instructionToString(inst);
    fprintf(out_code_file, "%s %s %s %s\n", instruction, op1, op2, op3);
    free(instruction);
}

void processInstruction(Instruction inst, char **operands, int operands_count)
{
    switch (operands_count)
    {
    case 0:
        handle_0_operand_instructions(inst);
        break;
    case 1:
        handle_1_operand_instructions(inst, operands[0]);
        break;
    case 2:
        handle_2_operand_instructions(inst, operands[0], operands[1]);
        break;
    case 3:
        handle_3_operand_instructions(inst, operands[0], operands[1], operands[2]);
        break;
    default:
        throw_error(INTERNAL_ERR, -1, "Invalid number of tokens\n");
        break;
    }
}

/// FORMATTING FUNCTIONS

char *symb_resolve(Token token)
{
    char *var_name = malloc(sizeof(char) * (10 + strlen(token.token_value)));
    switch (token.type)
    {
    case TOKEN_IDENTIFICATOR:
    {
        // for identificators, we need to find the variable in the symtable
        symtable_item *found = symtable_find_in_stack(token.token_value, sym_st, false);
        if (found == NULL)
        {
            // if the symbol has not been found, we we will presume that it's just temporary
            sprintf(var_name, "%s@$%s", "TF", token.token_value);
            break;
        }
        symtable_item item = *found;
        if (item.type == VARIABLE)
        {
            // check if the variable is initialized
            if (item.data.var_data->type == TYPE_INVALID)
            {
                throw_error(INTERNAL_ERR, -1, "Variable '%s' is invalid\n", token.token_value);
            }
            sprintf(var_name, "%s@$%s%d", item.scope == 0 ? "GF" : (item.scope < 0 ? "TF" : "LF"), item.id,
                    item.scope);
            break;
        }
        else if (item.type == FUNCTION)
        {
            // functions are just labels and function names are already uniques
            sprintf(var_name, "%s", item.id);
            break;
        }
        else
        {
            // this state should never occur
            sprintf(var_name, "%s", token.token_value);
            break;
        }
        sprintf(var_name, "%s@$%s%d", item.scope == 0 ? "GF" : (item.scope < 0 ? "TF" : "LF"), item.id, (int)item.data.var_data->gen_id_idx);
        break;
    }
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
    case TOKEN_BOOL:
    {
        // literals need to be formatted based on their type
        free(var_name);
        var_name = format_token(token);
        break;
    }
    default:
        sprintf(var_name, "%s", token.token_value);
        break;
    }
    return var_name;
}

char *format_token(Token token)
{
    char *formatted_value = NULL;

    switch (token.type)
    {
    case TOKEN_IDENTIFICATOR:
        throw_error(INTERNAL_ERR, -1, "Identificator\n");
        break;
    case TOKEN_INT:
    {
        // Format integer literals with "int@"
        formatted_value = malloc(strlen(token.token_value) + 5); //"int@" and '\0'
        sprintf(formatted_value, "int@%s", token.token_value);
        break;
    }
    case TOKEN_DOUBLE:
    {
        // Format floating-point literals with "float@"
        double double_value = atof(token.token_value); // Convert to double
        formatted_value = malloc(sizeof(char) * 60);   // Allocating enough space
        sprintf(formatted_value, "float@%a", double_value);
        break;
    }
    case TOKEN_STRING:
    {
        // Format string literals with "string@"
        formatted_value = malloc((strlen(token.token_value) * 10) + 8); //"string@" and '\0'
        sprintf(formatted_value, "string@%s", escapeString(token.token_value));
        break;
    }
    case TOKEN_BOOL:
    {
        // Format bool literals with "bool@"
        formatted_value = malloc(strlen(token.token_value) + 6); //"bool@" and '\0'
        sprintf(formatted_value, "bool@%s", token.token_value);
        break;
    }
    case TOKEN_NIL:
    {
        // Format nil with "nil@"
        formatted_value = malloc(strlen("nil@nil") + 1); //"nil@" and '\0'
        sprintf(formatted_value, "nil@nil");
        break;
    }
    default:
    {
        // Format other tokens with their value
        formatted_value = malloc(strlen(token.token_value) + 1);
        sprintf(formatted_value, "%s", token.token_value);
        break;
    }
    }
    return formatted_value;
}

void print_out_code()
{
    // Check if out_code_file is NULL
    if (out_code_file == NULL)
    {
        throw_error(INTERNAL_ERR, -1, "Error: out_code_file is not initialized.\n");
        return;
    }

    // Reset the file position to the beginning of the file
    if (fseek(out_code_file, 0, SEEK_SET) != 0)
    {
        throw_error(INTERNAL_ERR, -1, "Error: Failed to seek in out_code_file.\n");
        return;
    }

    int c; // fgetc returns int, not char
    while ((c = fgetc(out_code_file)) != EOF)
    {
        printf("%c", (char)c);
    }
}

/// GENERATION FUNCTIONS

void generate_func_header(symtable_item func_item)
{
    // CREATE OPERANDS
    char *func_lbl = malloc(sizeof(char) * (strlen(func_item.id) + 5));
    strcpy(func_lbl, func_item.id);

    char *func_end_lbl = malloc(sizeof(char) * (strlen(func_item.id) + 5));
    sprintf(func_end_lbl, "%s_end", func_item.id);

    generate_instruction(JUMP, label(func_end_lbl)); // jump over the function body
    generate_instruction(LABEL, label(func_lbl));    // function label

    fprintf(out_code_file, "\n");

    // frame initialization
    generate_instruction(PUSHFRAME);
    generate_instruction(CREATEFRAME);

    // initialize function params (in reverse order, because of stack)
    fprintf(out_code_file, "# function params\n");
    for (int i = func_item.data.func_data->params_count - 1; i >= 0; i--)
    {
        Token param = (Token){
            .type = TOKEN_IDENTIFICATOR,
            .token_value = malloc(sizeof(char) * (strlen(func_item.data.func_data->params[i].id) + 1)),
        };
        strcpy(param.token_value, func_item.data.func_data->params[i].id);

        char *var_name = symbol(param);

        HANDLE_DEFVAR(generate_instruction(DEFVAR, var_name););
        generate_instruction(POPS, var_name);

        free(var_name);
    }

    fprintf(out_code_file, "# function params end\n\n");

    free(func_end_lbl);
    free(func_lbl);
}

void generate_func_end(symtable_item func_item)
{
    char *func_lbl;
    func_lbl = malloc(sizeof(char) * (strlen(func_item.id) + 5));
    sprintf(func_lbl, "%s_end", func_item.id);

    generate_instruction(LABEL, label(func_lbl)); // function end label
    fprintf(out_code_file, "\n");

    free(func_lbl);
}

void generate_builtin_func_call(Token func, int param_cnt)
{
    char *tmp_token = malloc(sizeof(char) * 20);
    sprintf(tmp_token, "tmp%d", tmp_counter);
    char *tmp_token_name = variable(tmp_token, -1, false);
    BuiltinFunc builting_inst = getBuiltInFunctionName(func);

    switch (builting_inst)
    {
    case B_WRITE:
        fprintf(out_code_file, "# WRITE\n");

        // all the parameters separately in reverse order (because of stack)
        for (int i = 0; i < param_cnt; i++)
        {
            sprintf(tmp_token, "tmp%d", tmp_counter + param_cnt - i - 1);
            char *tmp_token_name = variable(tmp_token, -1, false);

            HANDLE_DEFVAR(generate_instruction(DEFVAR, tmp_token_name););
            generate_instruction(POPS, tmp_token_name);

            // tmp_counter++;
            sprintf(tmp_token, "tmp%d", tmp_counter);
            tmp_token_name = variable(tmp_token, -1, false);
        }

        // write all the parameters separately
        for (int i = 0; i < param_cnt; i++)
        {
            sprintf(tmp_token, "tmp%d", tmp_counter);
            char *tmp_token_name = variable(tmp_token, -1, false);

            generate_instruction(WRITE, tmp_token_name);

            tmp_counter++;
            sprintf(tmp_token, "tmp%d", tmp_counter);
            tmp_token_name = variable(tmp_token, -1, false);
        }

        tmp_counter--;
        fprintf(out_code_file, "# WRITE END\n");
        fprintf(out_code_file, "\n");
        break;
    case B_READ:
        // read the input and push it on the stack
        fprintf(out_code_file, "# READ\n");
        HANDLE_DEFVAR(generate_instruction(DEFVAR, tmp_token_name););
        generate_instruction(READ, tmp_token_name, type(getReadType(func))); // the type is determined by the function name
        generate_instruction(PUSHS, tmp_token_name);
        fprintf(out_code_file, "# READ END\n");
        fprintf(out_code_file, "\n");
        break;
    case B_INT2DOUBLE:
        // just a stack instruction
        fprintf(out_code_file, "# INT2DOUBLE\n");
        generate_instruction(INT2FLOATS);
        fprintf(out_code_file, "# INT2DOUBLE END\n");
        fprintf(out_code_file, "\n");
        break;
    case B_DOUBLE2INT:
        // just a stack instruction
        fprintf(out_code_file, "# FLOAT2INT\n");
        generate_instruction(FLOAT2INTS);
        fprintf(out_code_file, "# FLOAT2INT END\n");
        fprintf(out_code_file, "\n");
        break;
    case B_LENGTH:
    {
        fprintf(out_code_file, "# LENGTH\n");
        sprintf(tmp_token, "tmp%d", tmp_counter);

        tmp_counter++;

        sprintf(tmp_token, "tmp%d", tmp_counter);
        char *tmp_token_name_2 = variable(tmp_token, -1, false);
        tmp_counter++;

        // get the string to a temporary variable
        HANDLE_DEFVAR(generate_instruction(DEFVAR, tmp_token_name););
        HANDLE_DEFVAR(generate_instruction(DEFVAR, tmp_token_name_2););
        generate_instruction(POPS, tmp_token_name_2);
        // get the length of the string
        generate_instruction(STRLEN, tmp_token_name, tmp_token_name_2);
        // push the length on the stack
        generate_instruction(PUSHS, tmp_token_name);

        fprintf(out_code_file, "# LENGTH END\n");
        fprintf(out_code_file, "\n");
        break;
    }
    case B_SUBSTRING:
    {
        fprintf(out_code_file, "# SUBSTRING\n");

        // end index variable
        char *end_name = malloc(sizeof(char) * 20);
        sprintf(end_name, "tmp%d", tmp_counter);
        char *end = variable(end_name, -1, false);
        tmp_counter++;

        // start index variable
        char *start_name = malloc(sizeof(char) * 20);
        sprintf(start_name, "tmp%d", tmp_counter);
        char *start = variable(start_name, -1, false);
        tmp_counter++;

        // string variable
        char *string_name = malloc(sizeof(char) * 20);
        sprintf(string_name, "tmp%d", tmp_counter);
        char *string = variable(string_name, -1, false);
        tmp_counter++;

        // internal variable (used for storing the characters)
        char *internal_name = malloc(sizeof(char) * 20);
        sprintf(internal_name, "tmp%d", tmp_counter);
        char *internal = variable(internal_name, -1, false);
        tmp_counter++;

        // result variable (used for storing the resulting string)
        char *result_name = malloc(sizeof(char) * 20);
        sprintf(result_name, "tmp%d", tmp_counter);
        char *result = variable(result_name, -1, false);
        tmp_counter++;

        HANDLE_DEFVAR(generate_instruction(DEFVAR, end););
        HANDLE_DEFVAR(generate_instruction(DEFVAR, start););
        HANDLE_DEFVAR(generate_instruction(DEFVAR, string););
        HANDLE_DEFVAR(generate_instruction(DEFVAR, internal););
        HANDLE_DEFVAR(generate_instruction(DEFVAR, result););

        generate_instruction(POPS, end);
        generate_instruction(POPS, start);
        generate_instruction(POPS, string);

        generate_instruction(MOVE, result, literal((Token){
                                               .type = TOKEN_STRING,
                                               .token_value = "",
                                           }));

        char *loop_label = malloc(sizeof(char) * 20);
        sprintf(loop_label, "loop_substring_%d", tmp_counter);
        char *end_label = malloc(sizeof(char) * 20);
        sprintf(end_label, "end_substring_%d", tmp_counter);

        // loop until start != end
        generate_instruction(LABEL, label(loop_label));
        generate_instruction(JUMPIFEQ, label(end_label), start, end);

        // concat the character to the result
        generate_instruction(GETCHAR, internal, string, start);
        generate_instruction(CONCAT, result, result, internal);

        generate_instruction(ADD, start, start, literal((Token){
                                                    .type = TOKEN_INT,
                                                    .token_value = "1",
                                                }));
        generate_instruction(JUMP, label(loop_label));
        generate_instruction(LABEL, label(end_label));

        // push the resulting string on the stack
        generate_instruction(PUSHS, result);

        fprintf(out_code_file, "# SUBSTRING END\n");
        fprintf(out_code_file, "\n");

        tmp_counter++;

        break;
    }
    case B_ORD:
    {
        fprintf(out_code_file, "# STRI2INT\n");

        // push 0 as the index for the STRI2INTS instruction
        generate_instruction(PUSHS, literal((Token){
                                        .type = TOKEN_INT,
                                        .token_value = "0",
                                    }));
        generate_instruction(STRI2INTS);
        fprintf(out_code_file, "# STRI2INT END\n");
        fprintf(out_code_file, "\n");
        break;
    }
    case B_CHR:
    {
        // just a stack instruction
        fprintf(out_code_file, "# CHR\n");
        generate_instruction(INT2CHARS);
        fprintf(out_code_file, "# CHR END\n");
        fprintf(out_code_file, "\n");
        break;
    }
    default:
        throw_error(INTERNAL_ERR, -1, "Invalid built-in function.\n");
        break;
    }
    tmp_counter++;
}

void generate_if_start()
{
    if (else_label_st == NULL)
    {
        else_label_st = int_stack_init();
        int_stack_push(else_label_st, 0);
    }

    int elif_counter = int_stack_top(else_label_st);

    char *true_op = literal((Token){
        .type = TOKEN_BOOL,
        .token_value = "true",
    });

    char *if_lbl = malloc(sizeof(char) * 20);
    sprintf(if_lbl, "else%d_%d", if_counter, elif_counter);

    fprintf(out_code_file, "# if%d start\n", if_counter);

    generate_instruction(PUSHS, true_op); // push true to check the condition
    generate_instruction(JUMPIFNEQS, label(if_lbl));

    fprintf(out_code_file, "\n");

    free(if_lbl);

    if_counter++;
    int_stack_push(else_label_st, 0);
}

void generate_elseif_else()
{
    if_counter--;
    int elif_counter = 0;
    if (!int_stack_empty(else_label_st))
    {
        elif_counter = int_stack_pop(else_label_st);
    }

    char *endif_lbl = malloc(sizeof(char) * 10);
    sprintf(endif_lbl, "else%d_%d", if_counter, elif_counter);

    char *elsif_else_lbl = malloc(sizeof(char) * 20);
    sprintf(elsif_else_lbl, "else%d_%d", if_counter, elif_counter);

    generate_instruction(JUMP, label(endif_lbl)); // jump to end of if block
    generate_instruction(LABEL, label(elsif_else_lbl));

    fprintf(out_code_file, "\n");

    elif_counter++;

    free(endif_lbl);
    free(elsif_else_lbl);

    int_stack_push(else_label_st, elif_counter);
    if_counter++;
}

void generate_elseif_if()
{
    if_counter--;
    int elif_counter = 0;
    if (!int_stack_empty(else_label_st))
    {
        elif_counter = int_stack_pop(else_label_st);
    }

    char *true_op = literal((Token){
        .type = TOKEN_BOOL,
        .token_value = "true",
    });

    char *elsif_if_lbl = malloc(sizeof(char) * 20);
    sprintf(elsif_if_lbl, "else%d_%d", if_counter, elif_counter);

    fprintf(out_code_file, "# elseif%d start\n", elif_counter);

    generate_instruction(PUSHS, true_op); // push true to check the condition
    generate_instruction(JUMPIFNEQS, label(elsif_if_lbl));

    fprintf(out_code_file, "\n");

    free(elsif_if_lbl);

    int_stack_push(else_label_st, elif_counter);
    if_counter++;
}

void generate_else()
{
    if_counter--;
    int elif_counter = 0;
    if (!int_stack_empty(else_label_st))
    {
        elif_counter = int_stack_pop(else_label_st);
    }

    char *else_lbl = malloc(sizeof(char) * 10);
    sprintf(else_lbl, "else%d_%d", if_counter, elif_counter);

    elif_counter++;

    char *endif_lbl = malloc(sizeof(char) * 10);
    sprintf(endif_lbl, "else%d_%d", if_counter, elif_counter);

    fprintf(out_code_file, "# if%d else\n", if_counter);
    generate_instruction(JUMP, label(endif_lbl)); // jump to end of if block
    generate_instruction(LABEL, label(else_lbl));

    fprintf(out_code_file, "\n");

    free(else_lbl);
    free(endif_lbl);

    int_stack_push(else_label_st, elif_counter);
    if_counter++;
}

void generate_if_end()
{
    if_counter--;
    int elif_counter = 0;
    if (!int_stack_empty(else_label_st))
    {
        elif_counter = int_stack_pop(else_label_st);
    }

    fprintf(out_code_file, "# if%d end\n", if_counter);

    char *endif_lbl = malloc(sizeof(char) * 10);
    sprintf(endif_lbl, "else%d_%d", if_counter, elif_counter);
    generate_instruction(LABEL, label(endif_lbl)); // ending label of the if block

    fprintf(out_code_file, "\n");

    free(endif_lbl);
}

void generate_while_start()
{
    // just the label to jump to

    char *while_lbl = malloc(sizeof(char) * 10);
    sprintf(while_lbl, "while%d", while_counter);

    fprintf(out_code_file, "# while%d start\n", while_counter);

    // all the variable definitions should be above the looping while body, so we need to create another file just for the while body, save only the variable defenitions of the body into the original file and then copy the body file back to the original file
    while_def_out_code_file = out_code_file;
    out_code_file = tmpfile();
    if (out_code_file == NULL)
    {
        throw_error(INTERNAL_ERR, -1, "Error: out_code_file is not initialized.\n");
        return;
    }
    is_in_loop = true;

    generate_instruction(LABEL, label(while_lbl));

    fprintf(out_code_file, "\n");

    free(while_lbl);

    while_counter++;
}

void generate_while_condition()
{
    while_counter--;

    char *endwhile_lbl = malloc(sizeof(char) * 10);
    sprintf(endwhile_lbl, "endwhile%d", while_counter);

    char *true_op = literal((Token){
        .type = TOKEN_BOOL,
        .token_value = "true",
    });

    fprintf(out_code_file, "# while%d condition\n", while_counter);

    // check condition and jump accordingly
    generate_instruction(PUSHS, true_op);
    generate_instruction(JUMPIFNEQS, label(endwhile_lbl));

    fprintf(out_code_file, "\n");

    while_counter++;
}

void generate_while_end()
{
    while_counter--;

    // copy the while body back to the original file, swap the pointers back and delete the temporary file
    copyFileContents(out_code_file, while_def_out_code_file);
    out_code_file = while_def_out_code_file;
    is_in_loop = false;

    fprintf(out_code_file, "# while%d end\n", if_counter);

    char *endwhile_lbl = malloc(sizeof(char) * 10);
    sprintf(endwhile_lbl, "endwhile%d", while_counter);

    char *while_lbl = malloc(sizeof(char) * 10);
    sprintf(while_lbl, "while%d", while_counter);

    generate_instruction(JUMP, label(while_lbl));     // jump back to condition
    generate_instruction(LABEL, label(endwhile_lbl)); // while ending label

    fprintf(out_code_file, "\n");

    free(endwhile_lbl);
}

void generate_implicit_init(symtable_item var_item)
{
    if (var_item.data.var_data->is_initialized == false)
    {
        Expression_type type = var_item.data.var_data->type;
        if (type == TYPE_INT_NIL || type == TYPE_DOUBLE_NIL || type == TYPE_STRING_NIL || type == TYPE_BOOL_NIL)
        {
            char *nil = literal((Token){
                .type = TOKEN_NIL,
                .token_value = "nil",
            });
            generate_instruction(MOVE, variable(var_item.id, sym_st->size - 1, true), nil);
        }
    }
}

void generate_temp_pop()
{
    // create a temporary variable and pop the stack into it
    char *tmp_token = malloc(sizeof(char) * 20);
    sprintf(tmp_token, "tmp%d", tmp_counter);
    char *tmp_token_name = variable(tmp_token, -1, false);
    HANDLE_DEFVAR(generate_instruction(DEFVAR, tmp_token_name););
    generate_instruction(POPS, tmp_token_name);

    tmp_counter++;

    free(tmp_token);
}

void generate_temp_push()
{
    // push the value of the latest temporary value back onto the stack
    tmp_counter--;
    char *tmp_token = malloc(sizeof(char) * 20);
    sprintf(tmp_token, "tmp%d", tmp_counter);
    char *tmp_token_name = variable(tmp_token, -1, false);
    generate_instruction(PUSHS, tmp_token_name);

    tmp_counter++;

    free(tmp_token);
}

void generate_nil_coelacing()
{
    generate_temp_pop();
    generate_temp_pop();

    // operand deinitions

    char *tmp_token1 = malloc(sizeof(char) * 20);
    sprintf(tmp_token1, "tmp%d", tmp_counter - 2);
    char *tmp_token_name1 = variable(tmp_token1, -1, false);

    char *tmp_token2 = malloc(sizeof(char) * 20);
    sprintf(tmp_token2, "tmp%d", tmp_counter - 1);
    char *tmp_token_name2 = variable(tmp_token2, -1, false);

    char *not_nil_label = malloc(sizeof(char) * 20);
    sprintf(not_nil_label, "not_nil_%d", tmp_counter);

    char *was_nil_label = malloc(sizeof(char) * 20);
    sprintf(was_nil_label, "was_nil_%d", tmp_counter);

    // instruction generation

    // push left operand and nil
    generate_instruction(PUSHS, tmp_token_name2);
    generate_instruction(PUSHS, literal((Token){
                                    .type = TOKEN_NIL,
                                    .token_value = "nil",
                                }));
    // if left operand is nil, push right operand
    generate_instruction(JUMPIFNEQS, label(not_nil_label));
    // push right operand
    generate_instruction(PUSHS, tmp_token_name1);
    generate_instruction(JUMP, label(was_nil_label));
    generate_instruction(LABEL, label(not_nil_label));
    // else push left operand
    generate_instruction(PUSHS, tmp_token_name2);
    generate_instruction(LABEL, label(was_nil_label));
}

void generate_string_concat()
{
    generate_temp_pop();
    generate_temp_pop();

    // operand deinitions

    char *tmp_token1 = malloc(sizeof(char) * 20);
    sprintf(tmp_token1, "tmp%d", tmp_counter - 2);
    char *tmp_token_name1 = variable(tmp_token1, -1, false);

    char *tmp_token2 = malloc(sizeof(char) * 20);
    sprintf(tmp_token2, "tmp%d", tmp_counter - 1);
    char *tmp_token_name2 = variable(tmp_token2, -1, false);

    // concant the two strings and push the result
    generate_instruction(CONCAT, tmp_token_name1, tmp_token_name2, tmp_token_name1);
    generate_instruction(PUSHS, tmp_token_name1);
}

/// UTILITY FUNCTIONS
char *instructionToString(Instruction in)
{
    char *instruction = malloc(sizeof(char) * 100);
    switch (in)
    {
    case CREATEFRAME:
        sprintf(instruction, "CREATEFRAME");
        break;
    case PUSHFRAME:
        sprintf(instruction, "PUSHFRAME");
        break;
    case POPFRAME:
        sprintf(instruction, "POPFRAME");
        break;
    case RETURN:
        sprintf(instruction, "RETURN");
        break;
    case CLEARS:
        sprintf(instruction, "CLEARS");
        break;
    case ADDS:
        sprintf(instruction, "ADDS");
        break;
    case SUBS:
        sprintf(instruction, "SUBS");
        break;
    case DIVS:
        sprintf(instruction, "DIVS");
        break;
    case IDIVS:
        sprintf(instruction, "IDIVS");
        break;
    case MULS:
        sprintf(instruction, "MULS");
        break;
    case LTS:
        sprintf(instruction, "LTS");
        break;
    case EQS:
        sprintf(instruction, "EQS");
        break;
    case GTS:
        sprintf(instruction, "GTS");
        break;
    case ANDS:
        sprintf(instruction, "ANDS");
        break;
    case ORS:
        sprintf(instruction, "ORS");
        break;
    case NOTS:
        sprintf(instruction, "NOTS");
        break;
    case INT2FLOATS:
        sprintf(instruction, "INT2FLOATS");
        break;
    case FLOAT2INTS:
        sprintf(instruction, "FLOAT2INTS");
        break;
    case INT2CHARS:
        sprintf(instruction, "INT2CHARS");
        break;
    case STRI2INTS:
        sprintf(instruction, "STRI2INTS");
        break;
    case BREAK:
        sprintf(instruction, "BREAK");
        break;
    case CALL:
        sprintf(instruction, "CALL");
        break;
    case LABEL:
        sprintf(instruction, "LABEL");
        break;
    case JUMP:
        sprintf(instruction, "JUMP");
        break;
    case JUMPIFEQS:
        sprintf(instruction, "JUMPIFEQS");
        break;
    case JUMPIFNEQS:
        sprintf(instruction, "JUMPIFNEQS");
        break;
    case DEFVAR:
        sprintf(instruction, "DEFVAR");
        break;
    case POPS:
        sprintf(instruction, "POPS");
        break;
    case PUSHS:
        sprintf(instruction, "PUSHS");
        break;
    case WRITE:
        sprintf(instruction, "WRITE");
        break;
    case EXIT:
        sprintf(instruction, "EXIT");
        break;
    case DPRINT:
        sprintf(instruction, "DPRINT");
        break;
    case READ:
        sprintf(instruction, "READ");
        break;
    case MOVE:
        sprintf(instruction, "MOVE");
        break;
    case INT2FLOAT:
        sprintf(instruction, "INT2FLOAT");
        break;
    case FLOAT2INT:
        sprintf(instruction, "FLOAT2INT");
        break;
    case INT2CHAR:
        sprintf(instruction, "INT2CHAR");
        break;
    case STRI2INT:
        sprintf(instruction, "STRI2INT");
        break;
    case STRLEN:
        sprintf(instruction, "STRLEN");
        break;
    case TYPE:
        sprintf(instruction, "TYPE");
        break;
    case ADD:
        sprintf(instruction, "ADD");
        break;
    case SUB:
        sprintf(instruction, "SUB");
        break;
    case DIV:
        sprintf(instruction, "DIV");
        break;
    case IDIV:
        sprintf(instruction, "IDIV");
        break;
    case MUL:
        sprintf(instruction, "MUL");
        break;
    case LT:
        sprintf(instruction, "LT");
        break;
    case GT:
        sprintf(instruction, "GT");
        break;
    case EQ:
        sprintf(instruction, "EQ");
        break;
    case AND:
        sprintf(instruction, "AND");
        break;
    case OR:
        sprintf(instruction, "OR");
        break;
    case NOT:
        sprintf(instruction, "NOT");
        break;
    case CONCAT:
        sprintf(instruction, "CONCAT");
        break;
    case GETCHAR:
        sprintf(instruction, "GETCHAR");
        break;
    case SETCHAR:
        sprintf(instruction, "SETCHAR");
        break;
    case JUMPIFEQ:
        sprintf(instruction, "JUMPIFEQ");
        break;
    case JUMPIFNEQ:
        sprintf(instruction, "JUMPIFNEQ");
        break;
    default:
        sprintf(instruction, "UNKNOWN");
        break;
    }

    return instruction;
}

Instruction stringToInstruction(char *str)
{
    if (strcmp(str, "CREATEFRAME") == 0)
    {
        return CREATEFRAME;
    }
    else if (strcmp(str, "PUSHFRAME") == 0)
    {
        return PUSHFRAME;
    }
    else if (strcmp(str, "POPFRAME") == 0)
    {
        return POPFRAME;
    }
    else if (strcmp(str, "RETURN") == 0)
    {
        return RETURN;
    }
    else if (strcmp(str, "CLEARS") == 0)
    {
        return CLEARS;
    }
    else if (strcmp(str, "ADDS") == 0)
    {
        return ADDS;
    }
    else if (strcmp(str, "SUBS") == 0)
    {
        return SUBS;
    }
    else if (strcmp(str, "DIVS") == 0)
    {
        return DIVS;
    }
    else if (strcmp(str, "IDIVS") == 0)
    {
        return IDIVS;
    }
    else if (strcmp(str, "MULS") == 0)
    {
        return MULS;
    }
    else if (strcmp(str, "LTS") == 0)
    {
        return LTS;
    }
    else if (strcmp(str, "EQS") == 0)
    {
        return EQS;
    }
    else if (strcmp(str, "GTS") == 0)
    {
        return GTS;
    }
    else if (strcmp(str, "ANDS") == 0)
    {
        return ANDS;
    }
    else if (strcmp(str, "ORS") == 0)
    {
        return ORS;
    }
    else if (strcmp(str, "NOTS") == 0)
    {
        return NOTS;
    }
    else if (strcmp(str, "INT2FLOATS") == 0)
    {
        return INT2FLOATS;
    }
    else if (strcmp(str, "FLOAT2INTS") == 0)
    {
        return FLOAT2INTS;
    }
    else if (strcmp(str, "INT2CHARS") == 0)
    {
        return INT2CHARS;
    }
    else if (strcmp(str, "STRI2INTS") == 0)
    {
        return STRI2INTS;
    }
    else if (strcmp(str, "BREAK") == 0)
    {
        return BREAK;
    }
    else if (strcmp(str, "CALL") == 0)
    {
        return CALL;
    }
    else if (strcmp(str, "LABEL") == 0)
    {
        return LABEL;
    }
    else if (strcmp(str, "JUMP") == 0)
    {
        return JUMP;
    }
    else if (strcmp(str, "JUMPIFEQS") == 0)
    {
        return JUMPIFEQS;
    }
    else if (strcmp(str, "JUMPIFNEQS") == 0)
    {
        return JUMPIFNEQS;
    }
    else if (strcmp(str, "DEFVAR") == 0)
    {
        return DEFVAR;
    }
    else if (strcmp(str, "POPS") == 0)
    {
        return POPS;
    }
    else if (strcmp(str, "PUSHS") == 0)
    {
        return PUSHS;
    }
    else if (strcmp(str, "WRITE") == 0)
    {
        return WRITE;
    }
    else if (strcmp(str, "EXIT") == 0)
    {
        return EXIT;
    }
    else if (strcmp(str, "DPRINT") == 0)
    {
        return DPRINT;
    }
    else if (strcmp(str, "READ") == 0)
    {
        return READ;
    }
    else if (strcmp(str, "MOVE") == 0)
    {
        return MOVE;
    }
    else if (strcmp(str, "INT2FLOAT") == 0)
    {
        return INT2FLOAT;
    }
    else if (strcmp(str, "FLOAT2INT") == 0)
    {
        return FLOAT2INT;
    }
    else if (strcmp(str, "INT2CHAR") == 0)
    {
        return INT2CHAR;
    }
    else if (strcmp(str, "STRI2INT") == 0)
    {
        return STRI2INT;
    }
    else if (strcmp(str, "STRLEN") == 0)
    {
        return STRLEN;
    }
    else if (strcmp(str, "TYPE") == 0)
    {
        return TYPE;
    }
    else if (strcmp(str, "ADD") == 0)
    {
        return ADD;
    }
    else if (strcmp(str, "SUB") == 0)
    {
        return SUB;
    }
    else if (strcmp(str, "DIV") == 0)
    {
        return DIV;
    }
    else if (strcmp(str, "IDIV") == 0)
    {
        return IDIV;
    }
    else if (strcmp(str, "MUL") == 0)
    {
        return MUL;
    }
    else if (strcmp(str, "LT") == 0)
    {
        return LT;
    }
    else if (strcmp(str, "GT") == 0)
    {
        return GT;
    }
    else if (strcmp(str, "EQ") == 0)
    {
        return EQ;
    }
    else if (strcmp(str, "AND") == 0)
    {
        return AND;
    }
    else if (strcmp(str, "OR") == 0)
    {
        return OR;
    }
    else if (strcmp(str, "NOT") == 0)
    {
        return NOT;
    }
    else if (strcmp(str, "CONCAT") == 0)
    {
        return CONCAT;
    }
    else if (strcmp(str, "GETCHAR") == 0)
    {
        return GETCHAR;
    }
    else if (strcmp(str, "SETCHAR") == 0)
    {
        return SETCHAR;
    }
    else if (strcmp(str, "JUMPIFEQ") == 0)
    {
        return JUMPIFEQ;
    }
    else if (strcmp(str, "JUMPIFNEQ") == 0)
    {
        return JUMPIFNEQ;
    }
    return EMPTY;
}

bool isBuiltInFunction(Token token)
{

    if (strcmp(token.token_value, "readString") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "readInt") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "readDouble") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "write") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "Int2Double") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "Double2Int") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "length") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "substring") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "ord") == 0)
    {
        return true;
    }
    else if (strcmp(token.token_value, "chr") == 0)
    {
        return true;
    }

    return false;
}

BuiltinFunc getBuiltInFunctionName(Token token)
{
    if (strcmp(token.token_value, "readString") == 0)
    {
        return B_READ;
    }
    else if (strcmp(token.token_value, "readInt") == 0)
    {
        return B_READ;
    }
    else if (strcmp(token.token_value, "readDouble") == 0)
    {
        return B_READ;
    }
    else if (strcmp(token.token_value, "write") == 0)
    {
        return B_WRITE;
    }
    else if (strcmp(token.token_value, "Int2Double") == 0)
    {
        return B_INT2DOUBLE;
    }
    else if (strcmp(token.token_value, "Double2Int") == 0)
    {
        return B_DOUBLE2INT;
    }
    else if (strcmp(token.token_value, "length") == 0)
    {
        return B_LENGTH;
    }
    else if (strcmp(token.token_value, "substring") == 0)
    {
        return B_SUBSTRING;
    }
    else if (strcmp(token.token_value, "ord") == 0)
    {
        return B_ORD;
    }
    else if (strcmp(token.token_value, "chr") == 0)
    {
        return B_CHR;
    }
    else
    {
        throw_error(INTERNAL_ERR, -1, "wrong builtin function");
        return B_INVALID;
    }
}

Expression_type getReadType(Token token)
{
    if (strcmp(token.token_value, "readString") == 0)
    {
        return TYPE_STRING;
    }
    else if (strcmp(token.token_value, "readInt") == 0)
    {
        return TYPE_INT;
    }
    else if (strcmp(token.token_value, "readDouble") == 0)
    {
        return TYPE_DOUBLE;
    }
    return TYPE_INVALID;
}

char *escapeString(char *input)
{
    char *result = malloc(strlen(input) * 4 + 1); // Alokace paměti s dostatečnou kapacitou
    if (!result)
    {
        return NULL;
    }

    result[0] = '\0'; // Inicializace result jako prázdného řetězce
    int pos = 0;
    for (int i = 0; input[i] != '\0'; i++)
    {
        switch (input[i])
        {
        case '\0':
            pos += sprintf(result + pos, "\\000");
            break;
        case '\a':
            pos += sprintf(result + pos, "\\007");
            break;
        case '\b':
            pos += sprintf(result + pos, "\\008");
            break;
        case '\t':
            pos += sprintf(result + pos, "\\009");
            break;
        case '\n':
            pos += sprintf(result + pos, "\\010");
            break;
        case '\v':
            pos += sprintf(result + pos, "\\011");
            break;
        case '\f':
            pos += sprintf(result + pos, "\\012");
            break;
        case '\r':
            pos += sprintf(result + pos, "\\013");
            break;
        case ' ':
            pos += sprintf(result + pos, "\\032");
            break;
        case '#':
            pos += sprintf(result + pos, "\\035");
            break;
        case '\\':
            pos += sprintf(result + pos, "\\092");
            break;
        default:
            result[pos++] = input[i];
            result[pos] = '\0'; // Udržujeme řetězec ukončený
        }
    }
    return result;
}

void copyFileContents(FILE *source, FILE *destination)
{
    char ch;

    // Check if either file is NULL
    if (source == NULL || destination == NULL)
    {
        fprintf(stderr, "Invalid file pointer.\n");
        exit(EXIT_FAILURE);
    }

    // Rewind the source file to ensure it starts from the beginning
    rewind(source);

    fprintf(destination, "# ======== while ========\n");

    // Read from source and write to destination
    while ((ch = fgetc(source)) != EOF)
    {
        fputc(ch, destination);
    }

    fprintf(destination, "# ====== end while ======\n");
}