#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/thread.h"

void syscall_init (void);
void check_address (void* esp);
void check_address_byte (uint8_t * addr);
tid_t exec (const char *file_name);
int wait(tid_t child_tid);
void exit(int status);
void halt(void);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize(int fd);
int read(int fd, void* buffer, unsigned size);
int write(int fd, const void* buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

#endif /* userprog/syscall.h */
