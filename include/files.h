#ifndef _FILES_H_INCLUDED
#define _FILES_H_INCLUDED

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>

typedef char filename_t[NAME_MAX];
typedef const char *pathname_t;

/**
 * Is the full path of the app executable
 * @info do not need to be freed
 */
char *get_executable_path();
/**
 * Is the directory path of the app executable
 * @info do not need to be freed
 */
char *get_executable_dir();

char *file_path_concat(char *buffer, pathname_t base, pathname_t filepath);

//! Copy data of a file into another
/**
 * @param[in, out] in Input file
 * @param[in, out] out Output file
 */
int file_text_copy(FILE *out, FILE *in);

//! Copy data of a file into another using their pathnames
/**
 * @param[in] src_path Pathname of the input file
 * @param[out] dest_path Pathname of the output file
 */
int file_text_copy_from_pathnames(pathname_t dest_path, pathname_t src_path);

#endif
