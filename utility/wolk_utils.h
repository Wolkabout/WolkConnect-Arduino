#ifndef UTILS_H
#define UTILS_H

#define WOLK_UNUSED(x) ((void)x)
#define WOLK_ASSERT(x)

#define WOLK_ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#ifndef NULL
#  define NULL 0
#endif

#endif
