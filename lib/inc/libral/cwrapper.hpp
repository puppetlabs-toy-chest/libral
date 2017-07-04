#pragma once

#include "stdint.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* get_all(char* type_name_c);

struct outcome {
    char* result;
    uint8_t error_code;
};

uint8_t get_all_with_err(char **resource,
                         char *type_name);

uint8_t get_types(char *types);

struct outcome get_all_outcome(char* type_name_c);

#ifdef __cplusplus
}
#endif
