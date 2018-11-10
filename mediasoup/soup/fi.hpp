#ifndef FI_HPP
#define FI_HPP
#include "uv_callback.h"
extern uv_callback_t to_cpp;
uv_callback_t from_cpp;



#ifdef __cplusplus
extern "C"
{
#endif
//uv_callback_t from_cpp;
//void * on_from_cpp(uv_callback_t*,void*);
//void*set_channel(void);
//void soup_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
