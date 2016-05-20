//
// Created by Jarlene on 2016/5/4.
//

#ifndef CLASSPATCH_INLINEUTILS_H
#define CLASSPATCH_INLINEUTILS_H


#include <termios.h>

int find_name(pid_t pid, char *name, char *libn, unsigned long *addr);
int find_libbase(pid_t pid, char *libn, unsigned long *addr);




#endif //CLASSPATCH_INLINEUTILS_H
