#ifndef COMPILE_H
#define COMPILE_H

int cli(char* command, char* filename);
char* compile(char* filepath);
char* cleanup_build(char* filepath);

#endif