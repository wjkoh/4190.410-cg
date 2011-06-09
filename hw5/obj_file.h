#ifndef _OBJ_FILE_H_
#define _OBJ_FILE_H_

#include "common.h"

class triangle;
std::vector<std::shared_ptr<triangle>> read_obj_file(const std::string& fname);

#endif // _OBJ_FILE_H_
