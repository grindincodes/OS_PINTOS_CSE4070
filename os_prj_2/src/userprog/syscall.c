#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "threads/synch.h"
#include "syscall-nr.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
// #include "filesys/file.h"

struct lock lock;  // read, write lock. A single thread acquires and releases lock, and so on.

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{ 
  check_address(f->esp); // check stack pointer and exit if it is invalid
  
  uint32_t *args = (uint32_t *)(f->esp); // 4 byte 단위로 접근
  /* A Tip for Robustness of user-memory reference 
   여기서의 check_address와 read, write 내에서 check_address_byte로 buffer의 주소를 체크하는 것은 서로 다른 것이다.
   여기서의check_address는 args[1]과 같이 값을 읽기 전에 check_address로 값을 읽어와도 되는(유효한) 주소인지 확인하는 것이다.
   유저가 직접, 혹은 c언어의 user syscall api를 통해 인자를 쌓고 인터럽트 호출 -> 인터럽트 핸들러 -> 시스콜 핸들러-> open 수행 후 종료
   위의 과정에서 인자가 담긴 부분이 유효한 주소인지 확인한 후 args[1]과 같이 '값'을 읽어 전달하게 된다.
   만약 '값'이 포인터라면 create, read 등에서 따로 check_address_byte로 유효한 포인터인지 조사해야 한다.
   */
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
    case SYS_CREATE:
	    check_address(args + 1);
	    check_address(args + 2);
	    f->eax = create((const char*)args[1], (unsigned)args[2]);
	    break;
    case SYS_REMOVE:
	    check_address(args + 1);
	    f->eax = remove((const char*)args[1]);
	    break;
    case SYS_OPEN:
	    check_address(args + 1);
	    f->eax = open((const char*)args[1]);
	    break;
    case SYS_FILESIZE:
	    check_address(args + 1);
	    f->eax = filesize((int)args[1]);
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
    case SYS_SEEK:
	    check_address(args + 1);
	    check_address(args + 2);
	    seek((int)args[1], (unsigned)args[2]);
	    break;
    case SYS_TELL:
            check_address(args + 1);
	    f->eax = tell((int)args[1]);
	    break;
    case SYS_CLOSE:
	    check_address(args + 1);
	    close((int)args[1]);
	    break;
  }
}

/* check invalid pointer(esp),
   exit if invalid.
   user address is invalid, if it is(points)
   1. NULL (we should not dereference NULL), 
   2. ABOVE KERNEL vaddr,
   3. UNMAPPED user virtual memory .
   */
void
check_address (void* esp) {
  if (esp && (is_user_vaddr(esp)) && (pagedir_get_page(thread_current()->pagedir, esp)) ) {
    // not kernel vaddr(below PHYS_BASE) and not unmapped
    return;	 
  }
  else exit(-1);
}

/* check one single byte to check bad-ptr (user address provided by user)
   for char*, use uint8_t */
void
check_address_byte (uint8_t * addr) {
  check_address((void*)addr);
}

/* execute and return child pid
 * if load in child process failed, return -1  */
tid_t exec (const char *file_name) {
  check_address_byte((uint8_t *)file_name); // check address to the character buffer, provided by user.
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

bool
create (const char *file, unsigned initial_size) {
  if (file == NULL) exit(-1);
  check_address_byte((uint8_t *)file);
  return filesys_create(file, initial_size);
}

bool
remove (const char *file) {
  check_address_byte((uint8_t *)file);
  return filesys_remove(file);
}

int
open (const char *file) {
  if (file == NULL) exit(-1);
  check_address_byte((uint8_t *)file);
  int fd;
  struct file *fp = filesys_open(file);
  if (fp == NULL) {
    // file open error, no file named 'file' or allocation fail
    return -1;
  }
  else {
    fd = new_file(fp);
    if (fd == -1) return -1; // fd assign fail
    if (!strcmp(file, thread_current()->name))
      file_deny_write(fp); 	// no write to current executable file
    return fd;
  }
}

int filesize(int fd) {
  return file_length(fd_to_fp(fd));
}

int read(int fd, void* buffer, unsigned size) {
  char* buf = (char *)buffer;
  unsigned i;
  int size_read = -1;	// size that was read
  check_address_byte((uint8_t *)buffer);		// check buffer from start to end
  check_address_byte((uint8_t *)buffer + size - 1);

  if (fd == 0) { // read from stdin by size
    for (i=0; i<size; i++) {
      *buf = input_getc();
      if (*buf == '\0') {
        break;
      }
      buf += 1;
    }
    size_read = i;
  }
  else if (fd > 1){ // read from fd, if valid
    struct file * fp = fd_to_fp(fd);
    if (fp) { // if fp is not NULL
      /* file read critical section */
      lock_acquire(&lock);
      size_read = file_read(fp, buffer, size);
      lock_release(&lock);
      /* ... */
    }
    else exit(-1);	// bad-fd
  }
  else exit(-1);	// bad-fd
  return size_read;
}
int write(int fd, const void* buffer, unsigned size) {
  int size_written = -1;
  check_address_byte((uint8_t *)buffer);		// check buffer from start to end
  check_address_byte((uint8_t *)buffer + size - 1);
  if (fd == 1) {
    lock_acquire(&lock);
    putbuf((const char*)buffer, size);
    size_written = size;
    lock_release(&lock);
    return size_written;
  }
  else if (fd > 1) { // write to fd, if valid
    struct file * fp = fd_to_fp(fd);
    if (fp) {
      lock_acquire(&lock);
      size_written = file_write(fp, buffer, size);
      lock_release(&lock);
      return size_written;
    }
    else exit(-1);	// bad-fd
  }
  else exit(-1);	// bad-fd

  // return size_written;
}

void seek (int fd, unsigned position) {
  file_seek(fd_to_fp(fd), position);
}

unsigned tell (int fd) {
  return file_tell(fd_to_fp(fd));
}

void close (int fd) {
  if (fd <= 1) return;

  struct file * fp = fd_to_fp(fd);
  if (fp) {
    file_close(fp);
    thread_current() -> fd_table[fd] = NULL;
  }
  else exit(-1); // error if converted fp is NULL
}
