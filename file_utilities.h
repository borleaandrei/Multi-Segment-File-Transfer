//
// Created by sergiu on 3/3/18.
//

#ifndef MULTI_SEGMENT_FILE_TRANSFER_FILE_UTILITIES_H
#define MULTI_SEGMENT_FILE_TRANSFER_FILE_UTILITIES_H

#include <stdio.h>
#include <memory.h>

#define __USE_XOPEN_EXTENDED

#include <ftw.h>

#define DIR_PATH "./TEST_SERVER"
#define INDEX_FILE_NAME ".index"

int map_file_names_to_path(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

int create_index_file(void);

#endif //MULTI_SEGMENT_FILE_TRANSFER_FILE_UTILITIES_H
