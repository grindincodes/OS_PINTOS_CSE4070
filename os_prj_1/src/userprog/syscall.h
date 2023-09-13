#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/thread.h"

void syscall_init (void);
void check_address (void* esp);
tid_t exec (const char *file_name);
int wait(tid_t child_tid);
void exit(int status);
void halt(void);
int read(int fd, void* buffer,unsigned size);
int write(int fd, const void* buffer,unsigned size);

#endif /* userprog/syscall.h */
