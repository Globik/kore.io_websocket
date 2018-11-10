#ifndef _H_GLOBIKCOMMON
#define _H_GLOBIKCOMMON

#pragma once
#include <jansson.h>


json_t *load_json_buf(const char*, size_t);
json_t *load_json_str(const char*t);
#endif
