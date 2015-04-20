#pragma once

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

enum
{
	POINTER,
	NUMBER
};

#define FASTNET_OUT_OF_MEMORY 512
#define FASTNET_REGEX_INVALID_ID 513
#define FASTNET_REGEX_INVALID 514
#define FASTNET_REGEX_EMPTY 515
