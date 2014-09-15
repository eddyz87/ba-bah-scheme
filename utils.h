#ifndef __UTILS_H__
#define __UTILS_H__

#define ERROR(fmt, ...) error(__FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define ASSERT(cond, fmt, ...)                \
  do {                                        \
    if (!(cond)) { ERROR(fmt, ##__VA_ARGS__); } \
  } while(0)

void error(char *file, int line, char *fmt, ...);

#endif /*__UTILS_H__*/
