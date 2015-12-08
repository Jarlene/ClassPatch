//
// Created by Jarlene on 2015/9/29.
//

#ifndef COMMONDEMO_BASE64_H
#define COMMONDEMO_BASE64_H
#pragma once

#include <stdlib.h>

class base64 {
    int encode_base64 (const void *data, size_t length, char **code);
};


#endif //COMMONDEMO_BASE64_H
