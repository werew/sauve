#ifndef _COMMON_H_
#define _COMMON_H_


#define fail(x) _fail(x, __LINE__, __func__)
void _fail(const char* msg, int line, const char* func);


#endif
