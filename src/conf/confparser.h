// This file is autogenerated by VESC Tool

#ifndef CONFPARSER_H_
#define CONFPARSER_H_

#include "datatypes.h"
#include <stdint.h>
#include <stdbool.h>

// Constants
#define REFLOATCONFIG_SIGNATURE		3613224769

// Functions
int32_t confparser_serialize_refloatconfig(uint8_t *buffer, const RefloatConfig *conf);
bool confparser_deserialize_refloatconfig(const uint8_t *buffer, RefloatConfig *conf);
void confparser_set_defaults_refloatconfig(RefloatConfig *conf);

// CONFPARSER_H_
#endif
