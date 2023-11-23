#include "psa.h"

// TODO: handle implicit type conversion only for literals

PSA_Token getRule(PSA_Token *handle, unsigned int len)
{
    uint32_t handle_val = handleToUInt32(handle, len);
    /*
        E -> f
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
    case RULE_0:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> f\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[0].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_1a:
    case RULE_1b:
    case RULE_1c:
    case RULE_1d:
    case RULE_1e:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> i\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = getTypeFromToken(handle[0].type),
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_2:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> (E)\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_3:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> !E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_4:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> +E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_5:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> -E\n"););
        return (PSA_Token){
            .type = (Token_type)TOKEN_EXPRSN,
            .token_value = "E",
            .expr_type = handle[1].expr_type,
            .canBeNil = false, // TODO: get this from the symtable
        };
    case RULE_6:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E*E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_7:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E/E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_8:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E+E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_9:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E-E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_10:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E==E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_11:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E!=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_12:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E<E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_13:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E>E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_14:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E<=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_15:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E>=E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_16:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E&&E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_17:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E||E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    case RULE_18:
        DEBUG_PSA_CODE(printf_cyan("rule: E -> E??E\n"););
        return getHandleType(handle[0], handle[1].type, handle[2]);
    default:
        DEBUG_PSA_CODE(printf_red("rule: EOF\n"););
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