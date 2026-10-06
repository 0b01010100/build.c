/*
############################################################################################################

___________________________________Build file for C rebuilding C____________________________________________

############################################################################################################
*/



/*
############################################################################################################

_________________________________________________DEFINES____________________________________________________

############################################################################################################
*/

#include <stddef.h>
#if defined(_WIN32)
#define PLATFORM_WIN32 1
#define PLATFORM_EXE_EXT ".exe"
#elif defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX 1 
#define PLATFORM_EXE_EXT ""
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#else
#error "This type of apple platform is not supported"
#endif
#else
#error "Platform not supported"
#endif

// COMPTILE TIME HELPERS

#define COMPTIME_TYPE_BYTE_COUNT(expr) sizeof(expr)
#define COMPTIME_INDEX_COUNT(str) (COMPTIME_TYPE_BYTE_COUNT(str) / COMPTIME_TYPE_BYTE_COUNT(str[0]))
#define COMPTIME_STRLIT_AND_LEN(str) str, COMPTIME_INDEX_COUNT(str)

// https://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define issigned(t) (((t)(-1)) < ((t) 0))
#define umaxof(t) (((0x1ULL << ((COMPTIME_TYPE_BYTE_COUNT(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((COMPTIME_TYPE_BYTE_COUNT(t) * 8ULL) - 4ULL)))

#define smaxof(t) (((0x1ULL << ((COMPTIME_TYPE_BYTE_COUNT(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0x7ULL << ((COMPTIME_TYPE_BYTE_COUNT(t) * 8ULL) - 4ULL)))

#define maxof(t) ((unsigned long long) (issigned(t) ? smaxof(t) : umaxof(t)))

// RUNTIME OVERLOADS

#ifndef RUNTIME_STR_LEN_OVERLOAD
#include <string.h>
#define RUNTIME_STR_LEN_OVERLOAD strlen
#endif

#ifndef RUNTIME_ALLOC_OVERLOAD
#include <stdlib.h>
#define RUNTIME_ALLOC_OVERLOAD malloc
#endif

#ifndef RUNTIME_DEALLOC_OVERLOAD
#include <stdlib.h>
#define RUNTIME_DEALLOC_OVERLOAD free
#endif

// RUNTIME HELPERS
#define RUNTIME_ALLOC RUNTIME_ALLOC_OVERLOAD
#define RUNTIME_DEALLOC RUNTIME_ALLOC_OVERLOAD

#define RUNTIME_STR_LEN RUNTIME_STR_LEN_OVERLOAD
#define RUNTIME_STR_AND_LEN(str) str, RUNTIME_STR_LEN(str)

struct file_handle {
  void* internal_hande;
  int is_valid;
};

struct file_handle platform_get_out_handle();
struct file_handle platform_get_err_handle();

// will alway open in binary for all platforms
#define FILE_OPEN_READ 1
#define FILE_OPEN_WRITE 2
int platform_file_open(struct file_handle* file, int how, char *file_path);
void platform_file_close(struct file_handle* file);
size_t platform_file_write(struct file_handle* file, char *bytes, size_t count);

size_t platform_file_size(struct file_handle* file);
void platform_file_delete(char *filepath);
int platform_file_last_mod(struct file_handle* file, size_t* mtime);

void platform_directory_delete_recursive(char* file);
void platform_directory_create(char* file);
void platform_file_rename(char* file);

#ifndef CALLBACK_WRITE
#define CALLBACK_WRITE write_callback
#endif

void write_callback(char* message, size_t count)
{
    static struct file_handle out_put;
    if(!out_put.is_valid){
      out_put = platform_get_out_handle();
    }
    platform_file_write(&out_put, message, count);
}

struct command_list {
  char *command_str;
  size_t command_strlen;
  char **command_vec;
  size_t command_veclen;
};

int build_command_list_from_arg_vec(char *args[], char seperator,
                                    struct command_list *command_list);

int execute_process(struct command_list command_list);


/*
############################################################################################################

______________________________________________USER BUILD SPACE______________________________________________

############################################################################################################
*/

// OPTIONAL: Include or Copy-Paste compile commands extenstion
//#include "compile_commands_gen.h"

//__________________________________________OTHER USER BUILD SPACE__________________________________________

#define BUILD_CC "clang"
#define BUILDER_CC "clang"



char* SRC = "examples/BogoSort.c";
char* TARGET = "BogoSort" PLATFORM_EXE_EXT;
char* CFLAGS = "-g -Wall -Werror";

int builder_run(int argc, char* argv[]){
  struct command_list command_list;
  CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("\n"));
  char * args[] = {BUILD_CC, CFLAGS, SRC, "-o", TARGET, NULL};
  build_command_list_from_arg_vec(args, ' ',
                                  &command_list);

  CALLBACK_WRITE(command_list.command_str, command_list.command_strlen - 1);

  //  compile_commands_gen_push(command_list);
  execute_process(command_list);
  return 0;
}









/*
############################################################################################################

____________________________________________REBUILD SELF____________________________________________

############################################################################################################
*/


#ifndef BUILDER_CC
#error "Please selecte a compiler for the builder so it can rebuild it self"
#endif

// bool checks
int builder_check_needs_rebuild(void){
  // check file time deference between this file and the executables
  // todo
  return 1; // true
}

// bool checks
int build_builder(void){
// create <file-name>_old
// run <file-name>_old using execute porcess
// <file-name>_old wait untill this porcess is done.
// builds <file-name>

// the part is how can I delete <file-name>_old 
  return 1; // true
}

// int return code
int builder_setup(int argc, char* argv[]){
  if(builder_check_needs_rebuild()){
    if(!build_builder()){
      return 1; // EXIT_FAILURE if false
    }
  }


  return builder_run(argc, argv);
}


/*
############################################################################################################

____________________________________________HELPER FUNCTION DEFS____________________________________________

############################################################################################################
*/


int build_command_list_from_arg_vec(char *args[], char seperator,
                                    struct command_list *command_list) {
  command_list->command_veclen = 0;
  command_list->command_strlen = 0;

  char **args_it = &args[0];
  while (*args_it != NULL) {
    command_list->command_strlen += RUNTIME_STR_LEN(*args_it);
    command_list->command_veclen += 1;
    args_it++;
  }

  if (command_list->command_veclen > 0) {
    command_list->command_strlen += command_list->command_veclen - 1; // seperator
    command_list->command_strlen += 1; // '\0'
  } 

  command_list->command_str = RUNTIME_ALLOC(command_list->command_strlen * COMPTIME_TYPE_BYTE_COUNT(char));
  if (!command_list->command_str) {
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("failed memory allocate?"));
    return 0;
  }
  command_list->command_vec = RUNTIME_ALLOC((command_list->command_veclen)* COMPTIME_TYPE_BYTE_COUNT(char *));

  char *dest = command_list->command_str;
  for (size_t i = 0; i < command_list->command_veclen; i++) {
    command_list->command_vec[i] = dest;
    char *src = args[i];
    
    while (*src) {
      *dest++ = *src++;
    }

    if (i < command_list->command_veclen - 1) {
      *dest++ = seperator;
    } else {
      *dest++ = '\0';
    }
    // *p2++ = seperator;
    //CALLBACK_WRITE(RUNTIME_STR_AND_LEN(command_list->command_vec[i]));
    //CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("\n"));
  }
  command_list->command_vec[command_list->command_veclen] = NULL;
  return 1;
}




















































































































































/*
############################################################################################################

___________________________________________________PLATFORM SPACE__________________________________________

############################################################################################################
*/

// TODO: handle windows UNICODE
// this is the bane of my existence
// fucking WUTF16 and ANIS and the Ws and As

#if defined(PLATFORM_WIN32)
#include <Windows.h>

int main(int argc, char* argv[]){
  builder_setup(argc, argv);
}

size_t platform_file_write(struct file_handle* file, char *bytes, size_t count)
{
    DWORD bytes_to_write = count;
    DWORD bytes_written;
    
    WriteFile(file->internal_hande, bytes, bytes_to_write, &bytes_written, NULL);
    return bytes_to_write;
}

// https://learn.microsoft.com/en-us/windows/console/getstdhandle
struct file_handle platform_get_out_handle()
{
  struct file_handle handle = {0};
  HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (output_handle == INVALID_HANDLE_VALUE || output_handle == NULL)
  {
    handle.internal_hande = output_handle;
    handle.is_valid = 1;
    return  handle;
  }
  return handle;
}

struct file_handle platform_get_err_handle()
{
  struct file_handle handle = {0};
  HANDLE output_handle = GetStdHandle(STD_ERROR_HANDLE);
  if (output_handle == INVALID_HANDLE_VALUE || output_handle == NULL)
  {
    handle.internal_hande = output_handle;
    handle.is_valid = 1;
    return  handle;
  }
  return handle;
}

int platform_file_open(struct file_handle* file, int how, char *file_path)
{
  file->is_valid = 0;
  file->internal_hande = NULL;
  
  DWORD dwDesiredAccess = 0;
  if (how & FILE_OPEN_READ){
    dwDesiredAccess |= GENERIC_READ;
  }

  if (how & FILE_OPEN_WRITE){
    dwDesiredAccess |= GENERIC_WRITE;
  }

  /* Come back to the lpSecurityAttributes one*/
  HANDLE file_handle = CreateFileA(file_path, dwDesiredAccess, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(file_handle == NULL || file_handle == INVALID_HANDLE_VALUE){
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("CreateFileA Failed\n"));
    return -1;
  }

  file->internal_hande = file_handle;
  file->is_valid = 1;
}

void platform_file_close(struct file_handle* file)
{
  if(!file->is_valid || !file->internal_hande){return;}
  CloseHandle(file->internal_hande);
  file->internal_hande = NULL;
  file->is_valid = 0;
}

size_t platform_file_size(struct file_handle* file)
{
  if(!file->is_valid || !file->internal_hande){
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("platform_file_size: invalid handle\n"));
    return 0;
  }

  LARGE_INTEGER size;
  if(GetFileSizeEx(file->internal_hande, &size)){
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("platform_file_size->GetFileSizeEx\n"));
    return 0;
  }
  return size.QuadPart;
}

void platform_file_delete(char *filepath)
{
  DeleteFileA(filepath);
}

int platform_file_copy_to(char *src_path, char *dest_path){
  CopyFileExA(src_path, dest_path, NULL, NULL, FALSE, 0);
  // Possibly Memory map the file + using WriteFile instead of this ^
}

void platform_file_delete_if_exist(char *messaage)
{

}

void platform_directory_delete_recursive(char* file)
{

}
void platform_directory_create_if_not_exist(char* file)
{

}

int execute_process(struct command_list command_list) {
  STARTUPINFOA si = {0};
  PROCESS_INFORMATION pi = {0};

  si.cb = sizeof(si);
  if(!CreateProcessA(NULL, 
    command_list.command_str,
    NULL,
    NULL,
    FALSE,
    0,
    NULL,
    NULL,
    &si,
    &pi
  )){
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("CreateProcess Failed\n"));
    return 0;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  CloseHandle(&pi.dwThreadId);
  CloseHandle(&pi.dwProcessId);
  return 1;
}

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char* argv[]){
  return builder_setup(argc, argv);
}

void platform_write(char *messaage, size_t count) {
  write(STDOUT_FILENO, messaage, count);
}


struct file_handle platform_get_out_handle(){
  struct file_handle handle;
  handle.internal_hande = (void*)STDOUT_FILENO;
  handle.is_valid = 1;
  return handle;
}

struct file_handle platform_get_err_handle(){
  struct file_handle handle;
  handle.internal_hande = (void*)STDERR_FILENO;
  handle.is_valid = 1;
  return handle;
}

int platform_file_open(struct file_handle* file, int how, char *file_path)
{
  file->internal_hande = NULL;
  file->is_valid = 0;
  int oflag = O_CREAT;
  if(how & FILE_OPEN_READ && how & FILE_OPEN_WRITE){
      oflag |= O_RDWR;
  } else{
    if (how & FILE_OPEN_READ){
      oflag |= O_RDONLY;
    } else if (how & FILE_OPEN_WRITE) {
      oflag |= O_WRONLY;
    } else {
      //  
    }
  }
  int fd = open(file_path, oflag, 0644);
  if(fd < STDERR_FILENO){
    close(fd);
  }

  file->internal_hande = (void*)(size_t)fd;
  
}
void platform_file_close(struct file_handle *handle) {
    if (!handle || !handle->is_valid) return;

    int fd = (size_t)handle->internal_hande;

    if (fd > STDERR_FILENO) {
        close(fd);
    }

    handle->internal_hande = NULL;
    handle->is_valid = 0;
}

size_t platform_file_write(struct file_handle* file, char *bytes, size_t count)
{
  if(!file || !file->internal_hande) return -1;
  // TODO come back to this
  int fd = (int)(size_t)file->internal_hande;
  ssize_t result = write(fd, bytes, count);

  if(result < 0) return 0;
  return result;
}

size_t platform_file_size(struct file_handle* file)
{

}
void platform_file_delete(char *filepath)
{

}

int execute_process(struct command_list command_list) {
  // https://github.com/15b4t/ishell/blob/main/src/executor.c
  pid_t pid = fork();
  pid_t w;
  if (pid < 0) {
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("fork failed?\n"));
  }
  
  if (pid == 0) {
    execvp(command_list.command_vec[0], command_list.command_vec);
    CALLBACK_WRITE(COMPTIME_STRLIT_AND_LEN("execvp command formating error?\n"));
    //printf("I am a child process with id: %d\n", getpid());
    exit(127);
  }

  else if (pid > 0) {
    //printf("I am a parent process. the child has id: %d\n", pid);
    int status;
    w = waitpid(pid, &status, 0);
  }
  return 1;
}
#endif