/**
 * @file generator.c
 * @author Tomáš Hobza (xhobza03@vutbr.cz), Jakub Všetečka (xvsete00@vutbr.cz), Simona Valkovská <xvalko12@vutbr.cz>, Anastasia Butok <xbutok00@vutbr.cz>
 * @brief Generator of IFJcode20.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_label_instructions(Instruction inst)
{
    char *instruction = instructionToString(inst);
    fprintf(out_code_file, "%s\n", instruction);
    free(instruction);
}

void handle_var_instructions(Instruction inst, Token var);

void handle_symb_instructions(Instruction inst, Token symb);

void handle_var_symb_instructions(Instruction inst, Token var, Token symb);

void handle_var_symb_symb_instructions(Instruction inst, Token var, Token symb1, Token symb2);

void handle_var_type_instructions(Instruction inst, Token var, Token type);

void handle_no_operand_instructions(Instruction inst);

void handle_label_symb_symb_instructions(Instruction inst, Token label, Token symb1, Token symb2);

char *symb_resolve(Token *token)
{
    char *var_name = malloc(sizeof(char) * (10 + strlen(token->token_value)));
    switch (token->type)
    {
    case TOKEN_IDENTIFICATOR:
    {
        symtable_item item = *symtable_find_in_stack(token->token_value, sym_st, false);
        sprintf(var_name, "%s@$%s%d", item.scope == 0 ? "GF" : "LF", item.id, item.scope);
        break;
    }
    case TOKEN_STRING:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_NIL:
    {
        free(var_name);
        var_name = format_token(token);
        break;
    }
    default:
        throw_error(INTERNAL_ERR, -1, "Not valid\n");
        break;
    }
    return var_name;
}

char *format_token(Token *token)
{
    char *formatted_value = NULL;

    switch (token->type)
    {
    case TOKEN_IDENTIFICATOR:
        throw_error(INTERNAL_ERR, -1, "Identificator\n");
        break;
    case TOKEN_INT:
    {
        // Format integer literals with "int@"
        formatted_value = malloc(strlen(token->token_value) + 5); //"int@" and '\0'
        sprintf(formatted_value, "int@%s", token->token_value);
        break;
    }
    case TOKEN_DOUBLE:
    {
        // Format floating-point literals with "float@"
        double double_value = atof(token->token_value); // Convert to double
        formatted_value = malloc(sizeof(char) * 60);    // Allocating enough space
        sprintf(formatted_value, "float@%a", double_value);
        break;
    }
    case TOKEN_STRING:
    {
        // Format string literals with "string@"
        formatted_value = malloc(strlen(token->token_value) + 8); //"string@" and '\0'
        sprintf(formatted_value, "string@%s", token->token_value);
        break;
    }
    case TOKEN_BOOL:
    {
        // Format bool literals with "bool@"
        formatted_value = malloc(strlen(token->token_value) + 6); //"bool@" and '\0'
        sprintf(formatted_value, "bool@%s", token->token_value);
        break;
    }
    case TOKEN_NIL:
    {
        // Format nil with "nil@"
        formatted_value = malloc(strlen("nil@") + 1); //"nil@" and '\0'
        sprintf(formatted_value, "nil@");
        break;
    }
    default:
    {
        // Format other tokens with their value
        formatted_value = malloc(strlen(token->token_value) + 1);
        sprintf(formatted_value, "%s", token->token_value);
        break;
    }
    }
    return formatted_value;
}

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
