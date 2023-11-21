#ifndef UTILS_H
#define UTILS_H

#include "scanner.h"
#include "psa.h"
#include "colorful_printf.h"

/**
 * @brief Utility function for printing the token type.
 *
 * @param type
 */
void print_token_type(Token_type type);

/**
 * @brief Utility function for printing the expression type.
 *
 * @param type
 */
void print_expression_type(Expression_type type);

/**
 * @brief Converts a token to a PSA_Token.
 *
 * @param psa_tkn token to convert
 * @return Token - converted token
 */
Token convertPSATokenToToken(PSA_Token psa_tkn);

/**
 * @brief Converts a PSA_Token to a token.
 *
 * @param tkn token to convert
 * @return PSA_Token - converted token
 */
PSA_Token convertTokenToPSAToken(Token tkn);

#endif // UTILS_H