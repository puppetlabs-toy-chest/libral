#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

char* get_all(char* type_name_c);

uint8_t get_all_with_err(char *resource,
                         char *type_name);

uint8_t get_types(char *types);

#ifdef __cplusplus
}
#endif
