/**
 * @file generator.c
 * @author Tomáš Hobza (xhobza03@vutbr.cz), Jakub Všetečka (xvsete00@vutbr.cz)
 * @brief Generator of IFJcode20.
 * @version 0.1
 * @date 2023-11-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "generator.h"
// TODO: napsat funkci, ktera bere token (token.type je jeho typ a token.token_value je string s jeho hodnotou) a vraci string s jeho hodnotou ve formatu pro IFJcode23

void handle_label_instructions(Instruction inst);
void handle_var_instructions(Instruction inst, Token var);
void handle_symb_instructions(Instruction inst, Token symb);
void handle_var_symb_instructions(Instruction inst, Token var, Token symb);
void handle_var_symb_symb_instructions(Instruction inst, Token var, Token symb1, Token symb2);
void handle_var_type_instructions(Instruction inst, Token var, Token type);
void handle_no_operand_instructions(Instruction inst);
void handle_label_symb_symb_instructions(Instruction inst, Token label, Token symb1, Token symb2);

char *variable_to_ifjcode23(symtable_item *var)
{
    if (var == NULL || var->id == NULL)
    {
        throw_error(INTERNAL_ERR, -1, "Error: variable_to_ifjcode23 got NULL as an argument.\n");
        return NULL;
    }

    char *var_name = malloc(sizeof(char) * (10 + strlen(var->id)));
    sprintf(var_name, "%s@$%s%d", var->scope == 0 ? "GF" : "LF", var->id, var->scope);
    return var_name;
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

void assign(char *frame_var_to, char *name_var_to, char *frame_var_from, char *name_var_from) {
    THREE_INST("MOVE", frame_var_to, name_var_to, frame_var_from, name_var_from);
}

void add(var_frame,var_name,symb_type,symb_val,symb_type_2,symb_val_2){
    THREE_INST("ADD", var_fame, var_name, symb_val, name_var_from);
}

int main() {
    file = tmpfile();
    if (file == NULL) {
        fprintf(stderr, "Error creating file.\n");
        return INTERNAL_ERR;
    }

    fclose(file);
    return NO_ERR;
}
>>>>>>>>> Temporary merge branch 2
