#ifndef _SCANNER_H_
#define _SCANNER_H_


void* scanner(void* arg);
void launch_scanners(unsigned int n_scanners);

char* pop_folder();
void push_folder(char* folder);
void push_file(char* file);

void handle_file(const char* basedir, const char* filename);
void explore_folder(const char* folder);
void copy_dir(const char* src, const char* dest);

#endif 
