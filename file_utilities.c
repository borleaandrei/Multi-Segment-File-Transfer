//
// Created by sergiu on 3/3/18.
//

#include "file_utilities.h"

FILE *index_file;

/*Creates an index file via nftw() that stores file_name and file_name_path for easy file existance check
 * and file path retrieval(given file name)*/
int map_file_names_to_path(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    size_t line_len = 2*strlen(fpath) + 20;
    char file_name[line_len]; // +20 for the file size
    memset(file_name, '\0', line_len);

    // If fpath represents a normal file
    if(tflag == FTW_F) {
        // Construct "file_name file_name_path file_name_size"
        snprintf(file_name, line_len, "%s %s %d\n", fpath + ftwbuf->base, fpath, sb->st_size);

        // Write file_name and file_name_path into de index file
        fwrite((void *) file_name, sizeof(char), strlen(file_name) * sizeof(char), index_file);
    }

    // Do not stop until everything has been parsed
    return 0;
}

int create_index_file(void) {
    // Create index file (discarding whatever old information there is)
    index_file = fopen(INDEX_FILE_NAME, "w");
    if (index == NULL) {
        perror("Could not create index file!");
        fclose(index_file);
        return 1;
    }

    // Parse the entire DIR_PATH file tree, to a max depth of 20
    if (nftw(DIR_PATH, map_file_names_to_path, 20, 0) != 0) {
        perror("Could not parse the entire file tree!");
        fclose(index_file);
        return 1;
    }

    fclose(index_file);
    return 0;
}