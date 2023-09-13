#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "syscall-nr.h"
#include "devices/input.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ 
  check_address(f->esp); // check stack pointer and exit if it is invalid
  //check_address((uint32_t *)(f->esp) + 1);
  //check_address((uint32_t *)(f->esp) + 2);
  //check_address((uint32_t *)(f->esp) + 3);
  
  uint32_t *args = (uint32_t *)(f->esp); // 4 byte 단위로 접근
  switch (*(int *)(f->esp)) {
    case SYS_HALT:
	    halt();
	    break;
    case SYS_EXIT:
	    check_address(args +1);
	    exit((int)args[1]);
	    break;
    case SYS_EXEC:
	    check_address(args + 1);
	    f->eax = exec((const char *)args[1]);
	    break;
    case SYS_WAIT:
	    check_address(args + 1);
	    f->eax = wait((tid_t)args[1]);
	    break;
    case SYS_READ:
	    check_address(args + 1);
	    check_address(args + 2);
	    check_address(args + 3);
	    f->eax = read((int)args[1], (void *)args[2], (unsigned)args[3]);
	    break;
    case SYS_WRITE:
	    check_address(args + 1);
            check_address(args + 2);
            check_address(args + 3);
            f->eax = write((int)args[1], (const void *)args[2], (unsigned)args[3]);
	    break;
  }
}

/* check invalid pointer(esp) before dereferencing it. 
   exit if invalid */
void
check_address (void* esp) {
  if ( (is_user_vaddr(esp)) && (pagedir_get_page(thread_current()->pagedir, esp)) ) {
    // not kernel vaddr(below PHYS_BASE) and not unmapped
    return;	 
  }
  else exit(-1);
}

/* execute and return child pid
 * if load in child process failed, return -1  */
tid_t exec (const char *file_name) {
  tid_t child_tid = process_execute(file_name); // reaping은 언제?.. exec은 별개로  부모의 wait에 의해 reap되어야 한다.
  struct thread* ct = search_child(child_tid);
  sema_down(&ct->load_sema); // wait load
  if (ct->is_loaded) return child_tid;
  else return -1;
}

/* wait and return exit status */
int wait(tid_t child_tid) {
  return process_wait(child_tid);
}
void exit(int status) {
  struct thread *cur = thread_current();
  cur -> exit_status = status;
  printf("%s: exit(%d)\n", cur->name, cur->exit_status);
  thread_exit();
}
void halt() {
  shutdown_power_off();
}
int read(int fd, void* buffer,unsigned size) {
  unsigned count = size;
  char* buf = (char *)buffer;
  if (fd == 0) {
    while (count--) {
      *buf = input_getc();
      buf += 1;
    }
    return size;
  }
  return -1;  
}
int write(int fd, const void* buffer,unsigned size) {
  if (fd == 1) {
	putbuf((const char*)buffer, size);
  	return size;
  }
  return -1;
}
