#ifndef UTILS_H
#define UTILS_H
#define BUF_MAXLEN  100 // messages printed by unbuf_printf shouldn't go more than this length.

// async safe printf function to a fildes.
void unbuf_printf(int fildes, const char* format, ...);


#endif
