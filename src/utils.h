#pragma once

#include "npan.h"

#define LEFT_SHIFT(number, n) (number << (8 * n))
#define GET_TWO_BYTE(i) ((data[i] << 8) + data[i + 1])
#define GET_FOUR_BYTE(i) ((data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3])