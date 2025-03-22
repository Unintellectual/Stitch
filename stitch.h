#define STITCH_H_

#ifndef STITCH_ASSERT
#include <assert.h>
#define STITCH_ASSERT assert
#endif /* STITCH_ASSERT */

#ifndef STITCH_REALLOC
#include <stdlib.h>
#define STITCH_REALLOC realloc
#endif /* STITCH_REALLOC */

#ifndef STITCH_FREE
#include <stdlib.h>
#define STITCH_FREE free
#endif /* STITCH_FREE */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    define _WINUSER_
#    define _WINGDI_
#    define _IMM_
#    define _WINCON_
#    include <windows.h>
#    include <direct.h>
#    include <shellapi.h>
#else
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <fcntl.h>
#endif

#ifdef _WIN32
#    define STITCH_LINE_END "\r\n"
#else
#    define STITCH_LINE_END "\n"
#endif

#if defined(__GNUC__) || defined(__clang__)
// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#define STITCH_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#else
// TODO: implement STITCH_PRINTF_FORMAT for MSVC
#define STITCH_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

#define STITCH_UNUSED(value) (void)(value)
#define STITCH_TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define STITCH_UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

#define STITCH_ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))
#define STITCH_ARRAY_GET(array, index) \
    (STITCH_ASSERT((size_t)index < STITCH_ARRAY_LEN(array)), array[(size_t)index])

typedef enum {
    STITCH_INFO,
    STITCH_WARNING,
    STITCH_ERROR,
    STITCH_NO_LOGS,
} Stitch_Log_Level;

// Any messages with the level below stitch_minimal_log_level are going to be suppressed.
extern Stitch_Log_Level stitch_minimal_log_level;

void stitch_log(Stitch_Log_Level level, const char *fmt, ...) STITCH_PRINTF_FORMAT(2, 3);

// It is an equivalent of shift command from bash. It basically pops an element from
// the beginning of a sized array.
#define stitch_shift(xs, xs_sz) (STITCH_ASSERT((xs_sz) > 0), (xs_sz)--, *(xs)++)
// NOTE: stitch_shift_args() is an alias for an old variant of stitch_shift that only worked with
// the command line arguments passed to the main() function. stitch_shift() is more generic.
// So stitch_shift_args() is semi-deprecated, but I don't see much reason to urgently
// remove it. This alias does not hurt anybody.
#define stitch_shift_args(argc, argv) stitch_shift(*argv, *argc)

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Stitch_File_Paths;

typedef enum {
    STITCH_FILE_REGULAR = 0,
    STITCH_FILE_DIRECTORY,
    STITCH_FILE_SYMLINK,
    STITCH_FILE_OTHER,
} Stitch_File_Type;

bool stitch_mkdir_if_not_exists(const char *path);
bool stitch_copy_file(const char *src_path, const char *dst_path);
bool stitch_copy_directory_recursively(const char *src_path, const char *dst_path);
bool stitch_read_entire_dir(const char *parent, Stitch_File_Paths *children);
bool stitch_write_entire_file(const char *path, const void *data, size_t size);
Stitch_File_Type stitch_get_file_type(const char *path);
bool stitch_delete_file(const char *path);

#define stitch_return_defer(value) do { result = (value); goto defer; } while(0)

// Initial capacity of a dynamic array
#ifndef STITCH_DA_INIT_CAP
#define STITCH_DA_INIT_CAP 256
#endif

#define stitch_da_reserve(da, expected_capacity)                                              \
    do {                                                                                   \
        if ((expected_capacity) > (da)->capacity) {                                        \
            if ((da)->capacity == 0) {                                                     \
                (da)->capacity = STITCH_DA_INIT_CAP;                                          \
            }                                                                              \
            while ((expected_capacity) > (da)->capacity) {                                 \
                (da)->capacity *= 2;                                                       \
            }                                                                              \
            (da)->items = STITCH_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
            STITCH_ASSERT((da)->items != NULL && "Buy more RAM lol");                         \
        }                                                                                  \
    } while (0)

// Append an item to a dynamic array
#define stitch_da_append(da, item)                \
    do {                                       \
        stitch_da_reserve((da), (da)->count + 1); \
        (da)->items[(da)->count++] = (item);   \
    } while (0)

#define stitch_da_free(da) STITCH_FREE((da).items)

// Append several items to a dynamic array
#define stitch_da_append_many(da, new_items, new_items_count)                                      \
    do {                                                                                        \
        stitch_da_reserve((da), (da)->count + (new_items_count));                                  \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                       \
    } while (0)

#define stitch_da_resize(da, new_size)     \
    do {                                \
        stitch_da_reserve((da), new_size); \
        (da)->count = (new_size);       \
    } while (0)

#define stitch_da_last(da) (da)->items[(STITCH_ASSERT((da)->count > 0), (da)->count-1)]
#define stitch_da_remove_unordered(da, i)               \
    do {                                             \
        size_t j = (i);                              \
        STITCH_ASSERT(j < (da)->count);                 \
        (da)->items[j] = (da)->items[--(da)->count]; \
    } while(0)

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Stitch_String_Builder;

bool stitch_read_entire_file(const char *path, Stitch_String_Builder *sb);
int stitch_sb_appendf(Stitch_String_Builder *sb, const char *fmt, ...) STITCH_PRINTF_FORMAT(2, 3);

// Append a sized buffer to a string builder
#define stitch_sb_append_buf(sb, buf, size) stitch_da_append_many(sb, buf, size)

// Append a NULL-terminated string to a string builder
#define stitch_sb_append_cstr(sb, cstr)  \
    do {                              \
        const char *s = (cstr);       \
        size_t n = strlen(s);         \
        stitch_da_append_many(sb, s, n); \
    } while (0)

// Append a single NULL character at the end of a string builder. So then you can
// use it a NULL-terminated C string
#define stitch_sb_append_null(sb) stitch_da_append_many(sb, "", 1)

// Free the memory allocated by a string builder
#define stitch_sb_free(sb) STITCH_FREE((sb).items)

// Process handle
#ifdef _WIN32
typedef HANDLE Stitch_Proc;
#define STITCH_INVALID_PROC INVALID_HANDLE_VALUE
typedef HANDLE Stitch_Fd;
#define STITCH_INVALID_FD INVALID_HANDLE_VALUE
#else
typedef int Stitch_Proc;
#define STITCH_INVALID_PROC (-1)
typedef int Stitch_Fd;
#define STITCH_INVALID_FD (-1)
#endif // _WIN32

Stitch_Fd stitch_fd_open_for_read(const char *path);
Stitch_Fd stitch_fd_open_for_write(const char *path);
void stitch_fd_close(Stitch_Fd fd);

typedef struct {
    Stitch_Proc *items;
    size_t count;
    size_t capacity;
} Stitch_Procs;

bool stitch_procs_wait(Stitch_Procs procs);
bool stitch_procs_wait_and_reset(Stitch_Procs *procs);

// Wait until the process has finished
bool stitch_proc_wait(Stitch_Proc proc);

// A command - the main workhorse of Stitch. Stitch is all about building commands an running them
typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} Stitch_Cmd;

// Example:
// ```c
// Stitch_Fd fdin = stitch_fd_open_for_read("input.txt");
// if (fdin == STITCH_INVALID_FD) fail();
// Stitch_Fd fdout = stitch_fd_open_for_write("output.txt");
// if (fdout == STITCH_INVALID_FD) fail();
// Stitch_Cmd cmd = {0};
// stitch_cmd_append(&cmd, "cat");
// if (!stitch_cmd_run_sync_redirect_and_reset(&cmd, (Stitch_Cmd_Redirect) {
//     .fdin = &fdin,
//     .fdout = &fdout
// })) fail();
// ```
typedef struct {
    Stitch_Fd *fdin;
    Stitch_Fd *fdout;
    Stitch_Fd *fderr;
} Stitch_Cmd_Redirect;

// Render a string representation of a command into a string builder. Keep in mind the the
// string builder is not NULL-terminated by default. Use stitch_sb_append_null if you plan to
// use it as a C string.
void stitch_cmd_render(Stitch_Cmd cmd, Stitch_String_Builder *render);

#define stitch_cmd_append(cmd, ...) \
    stitch_da_append_many(cmd, \
                       ((const char*[]){__VA_ARGS__}), \
                       (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

#define stitch_cmd_extend(cmd, other_cmd) \
    stitch_da_append_many(cmd, (other_cmd)->items, (other_cmd)->count)

// Free all the memory allocated by command arguments
#define stitch_cmd_free(cmd) STITCH_FREE(cmd.items)

// Run command asynchronously
#define stitch_cmd_run_async(cmd) stitch_cmd_run_async_redirect(cmd, (Stitch_Cmd_Redirect) {0})
// NOTE: stitch_cmd_run_async_and_reset() is just like stitch_cmd_run_async() except it also resets cmd.count to 0
// so the Stitch_Cmd instance can be seamlessly used several times in a row
Stitch_Proc stitch_cmd_run_async_and_reset(Stitch_Cmd *cmd);
// Run redirected command asynchronously
Stitch_Proc stitch_cmd_run_async_redirect(Stitch_Cmd cmd, Stitch_Cmd_Redirect redirect);
// Run redirected command asynchronously and set cmd.count to 0 and close all the opened files
Stitch_Proc stitch_cmd_run_async_redirect_and_reset(Stitch_Cmd *cmd, Stitch_Cmd_Redirect redirect);

// Run command synchronously
bool stitch_cmd_run_sync(Stitch_Cmd cmd);
// NOTE: stitch_cmd_run_sync_and_reset() is just like stitch_cmd_run_sync() except it also resets cmd.count to 0
// so the Stitch_Cmd instance can be seamlessly used several times in a row
bool stitch_cmd_run_sync_and_reset(Stitch_Cmd *cmd);
// Run redirected command synchronously
bool stitch_cmd_run_sync_redirect(Stitch_Cmd cmd, Stitch_Cmd_Redirect redirect);
// Run redirected command synchronously and set cmd.count to 0 and close all the opened files
bool stitch_cmd_run_sync_redirect_and_reset(Stitch_Cmd *cmd, Stitch_Cmd_Redirect redirect);

#ifndef STITCH_TEMP_CAPACITY
#define STITCH_TEMP_CAPACITY (8*1024*1024)
#endif // STITCH_TEMP_CAPACITY
char *stitch_temp_strdup(const char *cstr);
void *stitch_temp_alloc(size_t size);
char *stitch_temp_sprintf(const char *format, ...) STITCH_PRINTF_FORMAT(1, 2);
void stitch_temp_reset(void);
size_t stitch_temp_save(void);
void stitch_temp_rewind(size_t checkpoint);

// Given any path returns the last part of that path.
// "/path/to/a/file.c" -> "file.c"; "/path/to/a/directory" -> "directory"
const char *stitch_path_name(const char *path);
bool stitch_rename(const char *old_path, const char *new_path);
int stitch_needs_rebuild(const char *output_path, const char **input_paths, size_t input_paths_count);
int stitch_needs_rebuild1(const char *output_path, const char *input_path);
int stitch_file_exists(const char *file_path);
const char *stitch_get_current_dir_temp(void);
bool stitch_set_current_dir(const char *path);

// TODO: add MinGW support for Go Rebuild Urself™ Technology
#ifndef STITCH_REBUILD_URSELF
#  if _WIN32
#    if defined(__GNUC__)
#       define STITCH_REBUILD_URSELF(binary_path, source_path) "gcc", "-o", binary_path, source_path
#    elif defined(__clang__)
#       define STITCH_REBUILD_URSELF(binary_path, source_path) "clang", "-o", binary_path, source_path
#    elif defined(_MSC_VER)
#       define STITCH_REBUILD_URSELF(binary_path, source_path) "cl.exe", stitch_temp_sprintf("/Fe:%s", (binary_path)), source_path
#    endif
#  else
#    define STITCH_REBUILD_URSELF(binary_path, source_path) "cc", "-o", binary_path, source_path
#  endif
#endif

// Go Rebuild Urself™ Technology
//
//   How to use it:
//     int main(int argc, char** argv) {
//         STITCH_GO_REBUILD_URSELF(argc, argv);
//         // actual work
//         return 0;
//     }
//
//   After your added this macro every time you run ./stitch it will detect
//   that you modified its original source code and will try to rebuild itself
//   before doing any actual work. So you only need to bootstrap your build system
//   once.
//
//   The modification is detected by comparing the last modified times of the executable
//   and its source code. The same way the make utility usually does it.
//
//   The rebuilding is done by using the STITCH_REBUILD_URSELF macro which you can redefine
//   if you need a special way of bootstraping your build system. (which I personally
//   do not recommend since the whole idea of NoBuild is to keep the process of bootstrapping
//   as simple as possible and doing all of the actual work inside of ./stitch)
//
void stitch__go_rebuild_urself(int argc, char **argv, const char *source_path, ...);
#define STITCH_GO_REBUILD_URSELF(argc, argv) stitch__go_rebuild_urself(argc, argv, __FILE__, NULL)
// Sometimes your stitch.c includes additional files, so you want the Go Rebuild Urself™ Technology to check
// if they also were modified and rebuild stitch.c accordingly. For that we have STITCH_GO_REBUILD_URSELF_PLUS():
// ```c
// #define STITCH_IMPLEMENTATION
// #include "stitch.h"
//
// #include "foo.c"
// #include "bar.c"
//
// int main(int argc, char **argv)
// {
//     STITCH_GO_REBUILD_URSELF_PLUS(argc, argv, "foo.c", "bar.c");
//     // ...
//     return 0;
// }
#define STITCH_GO_REBUILD_URSELF_PLUS(argc, argv, ...) stitch__go_rebuild_urself(argc, argv, __FILE__, __VA_ARGS__, NULL);

typedef struct {
    size_t count;
    const char *data;
} Stitch_String_View;

const char *stitch_temp_sv_to_cstr(Stitch_String_View sv);

Stitch_String_View stitch_sv_chop_by_delim(Stitch_String_View *sv, char delim);
Stitch_String_View stitch_sv_chop_left(Stitch_String_View *sv, size_t n);
Stitch_String_View stitch_sv_trim(Stitch_String_View sv);
Stitch_String_View stitch_sv_trim_left(Stitch_String_View sv);
Stitch_String_View stitch_sv_trim_right(Stitch_String_View sv);
bool stitch_sv_eq(Stitch_String_View a, Stitch_String_View b);
bool stitch_sv_end_with(Stitch_String_View sv, const char *cstr);
bool stitch_sv_starts_with(Stitch_String_View sv, Stitch_String_View expected_prefix);
Stitch_String_View stitch_sv_from_cstr(const char *cstr);
Stitch_String_View stitch_sv_from_parts(const char *data, size_t count);
// stitch_sb_to_sv() enables you to just view Stitch_String_Builder as Stitch_String_View
#define stitch_sb_to_sv(sb) stitch_sv_from_parts((sb).items, (sb).count)

// printf macros for String_View
#ifndef SV_Fmt
#define SV_Fmt "%.*s"
#endif // SV_Fmt
#ifndef SV_Arg
#define SV_Arg(sv) (int) (sv).count, (sv).data
#endif // SV_Arg
// USAGE:
//   String_View name = ...;
//   printf("Name: "SV_Fmt"\n", SV_Arg(name));



#ifndef _WIN32
#include <dirent.h>
#else // _WIN32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

struct dirent
{
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

static DIR *opendir(const char *dirpath);
static struct dirent *readdir(DIR *dirp);
static int closedir(DIR *dirp);

#endif // _WIN32

#ifdef _WIN32

char *stitch_win32_error_message(DWORD err);

#endif // _WIN32

#endif // STITCH_H_

#ifdef STITCH_IMPLEMENTATION

// Any messages with the level below stitch_minimal_log_level are going to be suppressed.
Stitch_Log_Level stitch_minimal_log_level = STITCH_INFO;

#ifdef _WIN32

// Base on https://stackoverflow.com/a/75644008
// > .NET Core uses 4096 * sizeof(WCHAR) buffer on stack for FormatMessageW call. And...thats it.
// >
// > https://github.com/dotnet/runtime/blob/3b63eb1346f1ddbc921374a5108d025662fb5ffd/src/coreclr/utilcode/posterror.cpp#L264-L265
#ifndef STITCH_WIN32_ERR_MSG_SIZE
#define STITCH_WIN32_ERR_MSG_SIZE (4 * 1024)
#endif // STITCH_WIN32_ERR_MSG_SIZE

char *stitch_win32_error_message(DWORD err) {
    static char win32ErrMsg[STITCH_WIN32_ERR_MSG_SIZE] = {0};
    DWORD errMsgSize = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, LANG_USER_DEFAULT, win32ErrMsg,
                                      STITCH_WIN32_ERR_MSG_SIZE, NULL);

    if (errMsgSize == 0) {
        if (GetLastError() != ERROR_MR_MID_NOT_FOUND) {
            if (sprintf(win32ErrMsg, "Could not get error message for 0x%lX", err) > 0) {
                return (char *)&win32ErrMsg;
            } else {
                return NULL;
            }
        } else {
            if (sprintf(win32ErrMsg, "Invalid Windows Error code (0x%lX)", err) > 0) {
                return (char *)&win32ErrMsg;
            } else {
                return NULL;
            }
        }
    }

    while (errMsgSize > 1 && isspace(win32ErrMsg[errMsgSize - 1])) {
        win32ErrMsg[--errMsgSize] = '\0';
    }

    return win32ErrMsg;
}

#endif // _WIN32

// The implementation idea is stolen from https://github.com/zhiayang/nabs
void stitch__go_rebuild_urself(int argc, char **argv, const char *source_path, ...)
{
    const char *binary_path = stitch_shift(argv, argc);
#ifdef _WIN32
    // On Windows executables almost always invoked without extension, so
    // it's ./stitch, not ./stitch.exe. For renaming the extension is a must.
    if (!stitch_sv_end_with(stitch_sv_from_cstr(binary_path), ".exe")) {
        binary_path = stitch_temp_sprintf("%s.exe", binary_path);
    }
#endif

    Stitch_File_Paths source_paths = {0};
    stitch_da_append(&source_paths, source_path);
    va_list args;
    va_start(args, source_path);
    for (;;) {
        const char *path = va_arg(args, const char*);
        if (path == NULL) break;
        stitch_da_append(&source_paths, path);
    }
    va_end(args);

    int rebuild_is_needed = stitch_needs_rebuild(binary_path, source_paths.items, source_paths.count);
    if (rebuild_is_needed < 0) exit(1); // error
    if (!rebuild_is_needed) {           // no rebuild is needed
        STITCH_FREE(source_paths.items);
        return;
    }

    Stitch_Cmd cmd = {0};

    const char *old_binary_path = stitch_temp_sprintf("%s.old", binary_path);

    if (!stitch_rename(binary_path, old_binary_path)) exit(1);
    stitch_cmd_append(&cmd, STITCH_REBUILD_URSELF(binary_path, source_path));
    if (!stitch_cmd_run_sync_and_reset(&cmd)) {
        stitch_rename(old_binary_path, binary_path);
        exit(1);
    }
#ifdef STITCH_EXPERIMENTAL_DELETE_OLD
    // TODO: this is an experimental behavior behind a compilation flag.
    // Once it is confirmed that it does not cause much problems on both POSIX and Windows
    // we may turn it on by default.
    stitch_delete_file(old_binary_path);
#endif // STITCH_EXPERIMENTAL_DELETE_OLD

    stitch_cmd_append(&cmd, binary_path);
    stitch_da_append_many(&cmd, argv, argc);
    if (!stitch_cmd_run_sync_and_reset(&cmd)) exit(1);
    exit(0);
}

static size_t stitch_temp_size = 0;
static char stitch_temp[STITCH_TEMP_CAPACITY] = {0};

bool stitch_mkdir_if_not_exists(const char *path)
{
#ifdef _WIN32
    int result = mkdir(path);
#else
    int result = mkdir(path, 0755);
#endif
    if (result < 0) {
        if (errno == EEXIST) {
            stitch_log(STITCH_INFO, "directory `%s` already exists", path);
            return true;
        }
        stitch_log(STITCH_ERROR, "could not create directory `%s`: %s", path, strerror(errno));
        return false;
    }

    stitch_log(STITCH_INFO, "created directory `%s`", path);
    return true;
}

bool stitch_copy_file(const char *src_path, const char *dst_path)
{
    stitch_log(STITCH_INFO, "copying %s -> %s", src_path, dst_path);
#ifdef _WIN32
    if (!CopyFile(src_path, dst_path, FALSE)) {
        stitch_log(STITCH_ERROR, "Could not copy file: %s", stitch_win32_error_message(GetLastError()));
        return false;
    }
    return true;
#else
    int src_fd = -1;
    int dst_fd = -1;
    size_t buf_size = 32*1024;
    char *buf = STITCH_REALLOC(NULL, buf_size);
    STITCH_ASSERT(buf != NULL && "Buy more RAM lol!!");
    bool result = true;

    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        stitch_log(STITCH_ERROR, "Could not open file %s: %s", src_path, strerror(errno));
        stitch_return_defer(false);
    }

    struct stat src_stat;
    if (fstat(src_fd, &src_stat) < 0) {
        stitch_log(STITCH_ERROR, "Could not get mode of file %s: %s", src_path, strerror(errno));
        stitch_return_defer(false);
    }

    dst_fd = open(dst_path, O_CREAT | O_TRUNC | O_WRONLY, src_stat.st_mode);
    if (dst_fd < 0) {
        stitch_log(STITCH_ERROR, "Could not create file %s: %s", dst_path, strerror(errno));
        stitch_return_defer(false);
    }

    for (;;) {
        ssize_t n = read(src_fd, buf, buf_size);
        if (n == 0) break;
        if (n < 0) {
            stitch_log(STITCH_ERROR, "Could not read from file %s: %s", src_path, strerror(errno));
            stitch_return_defer(false);
        }
        char *buf2 = buf;
        while (n > 0) {
            ssize_t m = write(dst_fd, buf2, n);
            if (m < 0) {
                stitch_log(STITCH_ERROR, "Could not write to file %s: %s", dst_path, strerror(errno));
                stitch_return_defer(false);
            }
            n    -= m;
            buf2 += m;
        }
    }

defer:
    STITCH_FREE(buf);
    close(src_fd);
    close(dst_fd);
    return result;
#endif
}

void stitch_cmd_render(Stitch_Cmd cmd, Stitch_String_Builder *render)
{
    for (size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.items[i];
        if (arg == NULL) break;
        if (i > 0) stitch_sb_append_cstr(render, " ");
        if (!strchr(arg, ' ')) {
            stitch_sb_append_cstr(render, arg);
        } else {
            stitch_da_append(render, '\'');
            stitch_sb_append_cstr(render, arg);
            stitch_da_append(render, '\'');
        }
    }
}

Stitch_Proc stitch_cmd_run_async_redirect(Stitch_Cmd cmd, Stitch_Cmd_Redirect redirect)
{
    if (cmd.count < 1) {
        stitch_log(STITCH_ERROR, "Could not run empty command");
        return STITCH_INVALID_PROC;
    }

    Stitch_String_Builder sb = {0};
    stitch_cmd_render(cmd, &sb);
    stitch_sb_append_null(&sb);
    stitch_log(STITCH_INFO, "CMD: %s", sb.items);
    stitch_sb_free(sb);
    memset(&sb, 0, sizeof(sb));

#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    // TODO: check for errors in GetStdHandle
    siStartInfo.hStdError = redirect.fderr ? *redirect.fderr : GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = redirect.fdout ? *redirect.fdout : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = redirect.fdin ? *redirect.fdin : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // TODO: use a more reliable rendering of the command instead of cmd_render
    // cmd_render is for logging primarily
    stitch_cmd_render(cmd, &sb);
    stitch_sb_append_null(&sb);
    BOOL bSuccess = CreateProcessA(NULL, sb.items, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
    stitch_sb_free(sb);

    if (!bSuccess) {
        stitch_log(STITCH_ERROR, "Could not create child process: %s", stitch_win32_error_message(GetLastError()));
        return STITCH_INVALID_PROC;
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if (cpid < 0) {
        stitch_log(STITCH_ERROR, "Could not fork child process: %s", strerror(errno));
        return STITCH_INVALID_PROC;
    }

    if (cpid == 0) {
        if (redirect.fdin) {
            if (dup2(*redirect.fdin, STDIN_FILENO) < 0) {
                stitch_log(STITCH_ERROR, "Could not setup stdin for child process: %s", strerror(errno));
                exit(1);
            }
        }

        if (redirect.fdout) {
            if (dup2(*redirect.fdout, STDOUT_FILENO) < 0) {
                stitch_log(STITCH_ERROR, "Could not setup stdout for child process: %s", strerror(errno));
                exit(1);
            }
        }

        if (redirect.fderr) {
            if (dup2(*redirect.fderr, STDERR_FILENO) < 0) {
                stitch_log(STITCH_ERROR, "Could not setup stderr for child process: %s", strerror(errno));
                exit(1);
            }
        }

        // NOTE: This leaks a bit of memory in the child process.
        // But do we actually care? It's a one off leak anyway...
        Stitch_Cmd cmd_null = {0};
        stitch_da_append_many(&cmd_null, cmd.items, cmd.count);
        stitch_cmd_append(&cmd_null, NULL);

        if (execvp(cmd.items[0], (char * const*) cmd_null.items) < 0) {
            stitch_log(STITCH_ERROR, "Could not exec child process: %s", strerror(errno));
            exit(1);
        }
        STITCH_UNREACHABLE("stitch_cmd_run_async_redirect");
    }

    return cpid;
#endif
}

Stitch_Proc stitch_cmd_run_async_and_reset(Stitch_Cmd *cmd)
{
    Stitch_Proc proc = stitch_cmd_run_async(*cmd);
    cmd->count = 0;
    return proc;
}

Stitch_Proc stitch_cmd_run_async_redirect_and_reset(Stitch_Cmd *cmd, Stitch_Cmd_Redirect redirect)
{
    Stitch_Proc proc = stitch_cmd_run_async_redirect(*cmd, redirect);
    cmd->count = 0;
    if (redirect.fdin) {
        stitch_fd_close(*redirect.fdin);
        *redirect.fdin = STITCH_INVALID_FD;
    }
    if (redirect.fdout) {
        stitch_fd_close(*redirect.fdout);
        *redirect.fdout = STITCH_INVALID_FD;
    }
    if (redirect.fderr) {
        stitch_fd_close(*redirect.fderr);
        *redirect.fderr = STITCH_INVALID_FD;
    }
    return proc;
}

Stitch_Fd stitch_fd_open_for_read(const char *path)
{
#ifndef _WIN32
    Stitch_Fd result = open(path, O_RDONLY);
    if (result < 0) {
        stitch_log(STITCH_ERROR, "Could not open file %s: %s", path, strerror(errno));
        return STITCH_INVALID_FD;
    }
    return result;
#else
    // https://docs.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Stitch_Fd result = CreateFile(
                    path,
                    GENERIC_READ,
                    0,
                    &saAttr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY,
                    NULL);

    if (result == INVALID_HANDLE_VALUE) {
        stitch_log(STITCH_ERROR, "Could not open file %s: %s", path, stitch_win32_error_message(GetLastError()));
        return STITCH_INVALID_FD;
    }

    return result;
#endif // _WIN32
}

Stitch_Fd stitch_fd_open_for_write(const char *path)
{
#ifndef _WIN32
    Stitch_Fd result = open(path,
                     O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (result < 0) {
        stitch_log(STITCH_ERROR, "could not open file %s: %s", path, strerror(errno));
        return STITCH_INVALID_FD;
    }
    return result;
#else
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;

    Stitch_Fd result = CreateFile(
                    path,                            // name of the write
                    GENERIC_WRITE,                   // open for writing
                    0,                               // do not share
                    &saAttr,                         // default security
                    OPEN_ALWAYS,                     // open always
                    FILE_ATTRIBUTE_NORMAL,           // normal file
                    NULL                             // no attr. template
                );

    if (result == INVALID_HANDLE_VALUE) {
        stitch_log(STITCH_ERROR, "Could not open file %s: %s", path, stitch_win32_error_message(GetLastError()));
        return STITCH_INVALID_FD;
    }

    return result;
#endif // _WIN32
}

void stitch_fd_close(Stitch_Fd fd)
{
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif // _WIN32
}

bool stitch_procs_wait(Stitch_Procs procs)
{
    bool success = true;
    for (size_t i = 0; i < procs.count; ++i) {
        success = stitch_proc_wait(procs.items[i]) && success;
    }
    return success;
}

bool stitch_procs_wait_and_reset(Stitch_Procs *procs)
{
    bool success = stitch_procs_wait(*procs);
    procs->count = 0;
    return success;
}

bool stitch_proc_wait(Stitch_Proc proc)
{
    if (proc == STITCH_INVALID_PROC) return false;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(
                       proc,    // HANDLE hHandle,
                       INFINITE // DWORD  dwMilliseconds
                   );

    if (result == WAIT_FAILED) {
        stitch_log(STITCH_ERROR, "could not wait on child process: %s", stitch_win32_error_message(GetLastError()));
        return false;
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(proc, &exit_status)) {
        stitch_log(STITCH_ERROR, "could not get process exit code: %s", stitch_win32_error_message(GetLastError()));
        return false;
    }

    if (exit_status != 0) {
        stitch_log(STITCH_ERROR, "command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);

    return true;
#else
    for (;;) {
        int wstatus = 0;
        if (waitpid(proc, &wstatus, 0) < 0) {
            stitch_log(STITCH_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                stitch_log(STITCH_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            stitch_log(STITCH_ERROR, "command process was terminated by signal %d", WTERMSIG(wstatus));
            return false;
        }
    }

    return true;
#endif
}

bool stitch_cmd_run_sync_redirect(Stitch_Cmd cmd, Stitch_Cmd_Redirect redirect)
{
    Stitch_Proc p = stitch_cmd_run_async_redirect(cmd, redirect);
    if (p == STITCH_INVALID_PROC) return false;
    return stitch_proc_wait(p);
}

bool stitch_cmd_run_sync(Stitch_Cmd cmd)
{
    Stitch_Proc p = stitch_cmd_run_async(cmd);
    if (p == STITCH_INVALID_PROC) return false;
    return stitch_proc_wait(p);
}

bool stitch_cmd_run_sync_and_reset(Stitch_Cmd *cmd)
{
    bool p = stitch_cmd_run_sync(*cmd);
    cmd->count = 0;
    return p;
}

bool stitch_cmd_run_sync_redirect_and_reset(Stitch_Cmd *cmd, Stitch_Cmd_Redirect redirect)
{
    bool p = stitch_cmd_run_sync_redirect(*cmd, redirect);
    cmd->count = 0;
    if (redirect.fdin) {
        stitch_fd_close(*redirect.fdin);
        *redirect.fdin = STITCH_INVALID_FD;
    }
    if (redirect.fdout) {
        stitch_fd_close(*redirect.fdout);
        *redirect.fdout = STITCH_INVALID_FD;
    }
    if (redirect.fderr) {
        stitch_fd_close(*redirect.fderr);
        *redirect.fderr = STITCH_INVALID_FD;
    }
    return p;
}

void stitch_log(Stitch_Log_Level level, const char *fmt, ...)
{
    if (level < stitch_minimal_log_level) return;

    switch (level) {
    case STITCH_INFO:
        fprintf(stderr, "[INFO] ");
        break;
    case STITCH_WARNING:
        fprintf(stderr, "[WARNING] ");
        break;
    case STITCH_ERROR:
        fprintf(stderr, "[ERROR] ");
        break;
    case STITCH_NO_LOGS: return;
    default:
        STITCH_UNREACHABLE("stitch_log");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

bool stitch_read_entire_dir(const char *parent, Stitch_File_Paths *children)
{
    bool result = true;
    DIR *dir = NULL;

    dir = opendir(parent);
    if (dir == NULL) {
        #ifdef _WIN32
        stitch_log(STITCH_ERROR, "Could not open directory %s: %s", parent, stitch_win32_error_message(GetLastError()));
        #else
        stitch_log(STITCH_ERROR, "Could not open directory %s: %s", parent, strerror(errno));
        #endif // _WIN32
        stitch_return_defer(false);
    }

    errno = 0;
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
        stitch_da_append(children, stitch_temp_strdup(ent->d_name));
        ent = readdir(dir);
    }

    if (errno != 0) {
        #ifdef _WIN32
        stitch_log(STITCH_ERROR, "Could not read directory %s: %s", parent, stitch_win32_error_message(GetLastError()));
        #else
        stitch_log(STITCH_ERROR, "Could not read directory %s: %s", parent, strerror(errno));
        #endif // _WIN32
        stitch_return_defer(false);
    }

defer:
    if (dir) closedir(dir);
    return result;
}

bool stitch_write_entire_file(const char *path, const void *data, size_t size)
{
    bool result = true;

    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        stitch_log(STITCH_ERROR, "Could not open file %s for writing: %s\n", path, strerror(errno));
        stitch_return_defer(false);
    }

    //           len
    //           v
    // aaaaaaaaaa
    //     ^
    //     data

    const char *buf = data;
    while (size > 0) {
        size_t n = fwrite(buf, 1, size, f);
        if (ferror(f)) {
            stitch_log(STITCH_ERROR, "Could not write into file %s: %s\n", path, strerror(errno));
            stitch_return_defer(false);
        }
        size -= n;
        buf  += n;
    }

defer:
    if (f) fclose(f);
    return result;
}

Stitch_File_Type stitch_get_file_type(const char *path)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        stitch_log(STITCH_ERROR, "Could not get file attributes of %s: %s", path, stitch_win32_error_message(GetLastError()));
        return -1;
    }

    if (attr & FILE_ATTRIBUTE_DIRECTORY) return STITCH_FILE_DIRECTORY;
    // TODO: detect symlinks on Windows (whatever that means on Windows anyway)
    return STITCH_FILE_REGULAR;
#else // _WIN32
    struct stat statbuf;
    if (stat(path, &statbuf) < 0) {
        stitch_log(STITCH_ERROR, "Could not get stat of %s: %s", path, strerror(errno));
        return -1;
    }

    if (S_ISREG(statbuf.st_mode)) return STITCH_FILE_REGULAR;
    if (S_ISDIR(statbuf.st_mode)) return STITCH_FILE_DIRECTORY;
    if (S_ISLNK(statbuf.st_mode)) return STITCH_FILE_SYMLINK;
    return STITCH_FILE_OTHER;
#endif // _WIN32
}

bool stitch_delete_file(const char *path)
{
    stitch_log(STITCH_INFO, "deleting %s", path);
#ifdef _WIN32
    if (!DeleteFileA(path)) {
        stitch_log(STITCH_ERROR, "Could not delete file %s: %s", path, stitch_win32_error_message(GetLastError()));
        return false;
    }
    return true;
#else
    if (remove(path) < 0) {
        stitch_log(STITCH_ERROR, "Could not delete file %s: %s", path, strerror(errno));
        return false;
    }
    return true;
#endif // _WIN32
}

bool stitch_copy_directory_recursively(const char *src_path, const char *dst_path)
{
    bool result = true;
    Stitch_File_Paths children = {0};
    Stitch_String_Builder src_sb = {0};
    Stitch_String_Builder dst_sb = {0};
    size_t temp_checkpoint = stitch_temp_save();

    Stitch_File_Type type = stitch_get_file_type(src_path);
    if (type < 0) return false;

    switch (type) {
        case STITCH_FILE_DIRECTORY: {
            if (!stitch_mkdir_if_not_exists(dst_path)) stitch_return_defer(false);
            if (!stitch_read_entire_dir(src_path, &children)) stitch_return_defer(false);

            for (size_t i = 0; i < children.count; ++i) {
                if (strcmp(children.items[i], ".") == 0) continue;
                if (strcmp(children.items[i], "..") == 0) continue;

                src_sb.count = 0;
                stitch_sb_append_cstr(&src_sb, src_path);
                stitch_sb_append_cstr(&src_sb, "/");
                stitch_sb_append_cstr(&src_sb, children.items[i]);
                stitch_sb_append_null(&src_sb);

                dst_sb.count = 0;
                stitch_sb_append_cstr(&dst_sb, dst_path);
                stitch_sb_append_cstr(&dst_sb, "/");
                stitch_sb_append_cstr(&dst_sb, children.items[i]);
                stitch_sb_append_null(&dst_sb);

                if (!stitch_copy_directory_recursively(src_sb.items, dst_sb.items)) {
                    stitch_return_defer(false);
                }
            }
        } break;

        case STITCH_FILE_REGULAR: {
            if (!stitch_copy_file(src_path, dst_path)) {
                stitch_return_defer(false);
            }
        } break;

        case STITCH_FILE_SYMLINK: {
            stitch_log(STITCH_WARNING, "TODO: Copying symlinks is not supported yet");
        } break;

        case STITCH_FILE_OTHER: {
            stitch_log(STITCH_ERROR, "Unsupported type of file %s", src_path);
            stitch_return_defer(false);
        } break;

        default: STITCH_UNREACHABLE("stitch_copy_directory_recursively");
    }

defer:
    stitch_temp_rewind(temp_checkpoint);
    stitch_da_free(src_sb);
    stitch_da_free(dst_sb);
    stitch_da_free(children);
    return result;
}

char *stitch_temp_strdup(const char *cstr)
{
    size_t n = strlen(cstr);
    char *result = stitch_temp_alloc(n + 1);
    STITCH_ASSERT(result != NULL && "Increase STITCH_TEMP_CAPACITY");
    memcpy(result, cstr, n);
    result[n] = '\0';
    return result;
}

void *stitch_temp_alloc(size_t size)
{
    if (stitch_temp_size + size > STITCH_TEMP_CAPACITY) return NULL;
    void *result = &stitch_temp[stitch_temp_size];
    stitch_temp_size += size;
    return result;
}

char *stitch_temp_sprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    STITCH_ASSERT(n >= 0);
    char *result = stitch_temp_alloc(n + 1);
    STITCH_ASSERT(result != NULL && "Extend the size of the temporary allocator");
    // TODO: use proper arenas for the temporary allocator;
    va_start(args, format);
    vsnprintf(result, n + 1, format, args);
    va_end(args);

    return result;
}

void stitch_temp_reset(void)
{
    stitch_temp_size = 0;
}

size_t stitch_temp_save(void)
{
    return stitch_temp_size;
}

void stitch_temp_rewind(size_t checkpoint)
{
    stitch_temp_size = checkpoint;
}

const char *stitch_temp_sv_to_cstr(Stitch_String_View sv)
{
    char *result = stitch_temp_alloc(sv.count + 1);
    STITCH_ASSERT(result != NULL && "Extend the size of the temporary allocator");
    memcpy(result, sv.data, sv.count);
    result[sv.count] = '\0';
    return result;
}

int stitch_needs_rebuild(const char *output_path, const char **input_paths, size_t input_paths_count)
{
#ifdef _WIN32
    BOOL bSuccess;

    HANDLE output_path_fd = CreateFile(output_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if (output_path_fd == INVALID_HANDLE_VALUE) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (GetLastError() == ERROR_FILE_NOT_FOUND) return 1;
        stitch_log(STITCH_ERROR, "Could not open file %s: %s", output_path, stitch_win32_error_message(GetLastError()));
        return -1;
    }
    FILETIME output_path_time;
    bSuccess = GetFileTime(output_path_fd, NULL, NULL, &output_path_time);
    CloseHandle(output_path_fd);
    if (!bSuccess) {
        stitch_log(STITCH_ERROR, "Could not get time of %s: %s", output_path, stitch_win32_error_message(GetLastError()));
        return -1;
    }

    for (size_t i = 0; i < input_paths_count; ++i) {
        const char *input_path = input_paths[i];
        HANDLE input_path_fd = CreateFile(input_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (input_path_fd == INVALID_HANDLE_VALUE) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            stitch_log(STITCH_ERROR, "Could not open file %s: %s", input_path, stitch_win32_error_message(GetLastError()));
            return -1;
        }
        FILETIME input_path_time;
        bSuccess = GetFileTime(input_path_fd, NULL, NULL, &input_path_time);
        CloseHandle(input_path_fd);
        if (!bSuccess) {
            stitch_log(STITCH_ERROR, "Could not get time of %s: %s", input_path, stitch_win32_error_message(GetLastError()));
            return -1;
        }

        // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
        if (CompareFileTime(&input_path_time, &output_path_time) == 1) return 1;
    }

    return 0;
#else
    struct stat statbuf = {0};

    if (stat(output_path, &statbuf) < 0) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (errno == ENOENT) return 1;
        stitch_log(STITCH_ERROR, "could not stat %s: %s", output_path, strerror(errno));
        return -1;
    }
    int output_path_time = statbuf.st_mtime;

    for (size_t i = 0; i < input_paths_count; ++i) {
        const char *input_path = input_paths[i];
        if (stat(input_path, &statbuf) < 0) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            stitch_log(STITCH_ERROR, "could not stat %s: %s", input_path, strerror(errno));
            return -1;
        }
        int input_path_time = statbuf.st_mtime;
        // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
        if (input_path_time > output_path_time) return 1;
    }

    return 0;
#endif
}

int stitch_needs_rebuild1(const char *output_path, const char *input_path)
{
    return stitch_needs_rebuild(output_path, &input_path, 1);
}

const char *stitch_path_name(const char *path)
{
#ifdef _WIN32
    const char *p1 = strrchr(path, '/');
    const char *p2 = strrchr(path, '\\');
    const char *p = (p1 > p2)? p1 : p2;  // NULL is ignored if the other search is successful
    return p ? p + 1 : path;
#else
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
#endif // _WIN32
}

bool stitch_rename(const char *old_path, const char *new_path)
{
    stitch_log(STITCH_INFO, "renaming %s -> %s", old_path, new_path);
#ifdef _WIN32
    if (!MoveFileEx(old_path, new_path, MOVEFILE_REPLACE_EXISTING)) {
        stitch_log(STITCH_ERROR, "could not rename %s to %s: %s", old_path, new_path, stitch_win32_error_message(GetLastError()));
        return false;
    }
#else
    if (rename(old_path, new_path) < 0) {
        stitch_log(STITCH_ERROR, "could not rename %s to %s: %s", old_path, new_path, strerror(errno));
        return false;
    }
#endif // _WIN32
    return true;
}

bool stitch_read_entire_file(const char *path, Stitch_String_Builder *sb)
{
    bool result = true;

    FILE *f = fopen(path, "rb");
    if (f == NULL)                 stitch_return_defer(false);
    if (fseek(f, 0, SEEK_END) < 0) stitch_return_defer(false);
    long m = ftell(f);
    if (m < 0)                     stitch_return_defer(false);
    if (fseek(f, 0, SEEK_SET) < 0) stitch_return_defer(false);

    size_t new_count = sb->count + m;
    if (new_count > sb->capacity) {
        sb->items = STITCH_REALLOC(sb->items, new_count);
        STITCH_ASSERT(sb->items != NULL && "Buy more RAM lool!!");
        sb->capacity = new_count;
    }

    fread(sb->items + sb->count, m, 1, f);
    if (ferror(f)) {
        // TODO: Afaik, ferror does not set errno. So the error reporting in defer is not correct in this case.
        stitch_return_defer(false);
    }
    sb->count = new_count;

defer:
    if (!result) stitch_log(STITCH_ERROR, "Could not read file %s: %s", path, strerror(errno));
    if (f) fclose(f);
    return result;
}

int stitch_sb_appendf(Stitch_String_Builder *sb, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    // NOTE: the new_capacity needs to be +1 because of the null terminator.
    // However, further below we increase sb->count by n, not n + 1.
    // This is because we don't want the sb to include the null terminator. The user can always sb_append_null() if they want it
    stitch_da_reserve(sb, sb->count + n + 1);
    char *dest = sb->items + sb->count;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    va_end(args);

    sb->count += n;

    return n;
}

Stitch_String_View stitch_sv_chop_by_delim(Stitch_String_View *sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    Stitch_String_View result = stitch_sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return result;
}

Stitch_String_View stitch_sv_chop_left(Stitch_String_View *sv, size_t n)
{
    if (n > sv->count) {
        n = sv->count;
    }

    Stitch_String_View result = stitch_sv_from_parts(sv->data, n);

    sv->data  += n;
    sv->count -= n;

    return result;
}

Stitch_String_View stitch_sv_from_parts(const char *data, size_t count)
{
    Stitch_String_View sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

Stitch_String_View stitch_sv_trim_left(Stitch_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return stitch_sv_from_parts(sv.data + i, sv.count - i);
}

Stitch_String_View stitch_sv_trim_right(Stitch_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return stitch_sv_from_parts(sv.data, sv.count - i);
}

Stitch_String_View stitch_sv_trim(Stitch_String_View sv)
{
    return stitch_sv_trim_right(stitch_sv_trim_left(sv));
}

Stitch_String_View stitch_sv_from_cstr(const char *cstr)
{
    return stitch_sv_from_parts(cstr, strlen(cstr));
}

bool stitch_sv_eq(Stitch_String_View a, Stitch_String_View b)
{
    if (a.count != b.count) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

bool stitch_sv_end_with(Stitch_String_View sv, const char *cstr)
{
    size_t cstr_count = strlen(cstr);
    if (sv.count >= cstr_count) {
        size_t ending_start = sv.count - cstr_count;
        Stitch_String_View sv_ending = stitch_sv_from_parts(sv.data + ending_start, cstr_count);
        return stitch_sv_eq(sv_ending, stitch_sv_from_cstr(cstr));
    }
    return false;
}


bool stitch_sv_starts_with(Stitch_String_View sv, Stitch_String_View expected_prefix)
{
    if (expected_prefix.count <= sv.count) {
        Stitch_String_View actual_prefix = stitch_sv_from_parts(sv.data, expected_prefix.count);
        return stitch_sv_eq(expected_prefix, actual_prefix);
    }

    return false;
}

// RETURNS:
//  0 - file does not exists
//  1 - file exists
// -1 - error while checking if file exists. The error is logged
int stitch_file_exists(const char *file_path)
{
#if _WIN32
    // TODO: distinguish between "does not exists" and other errors
    DWORD dwAttrib = GetFileAttributesA(file_path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES;
#else
    struct stat statbuf;
    if (stat(file_path, &statbuf) < 0) {
        if (errno == ENOENT) return 0;
        stitch_log(STITCH_ERROR, "Could not check if file %s exists: %s", file_path, strerror(errno));
        return -1;
    }
    return 1;
#endif
}

const char *stitch_get_current_dir_temp(void)
{
#ifdef _WIN32
    DWORD nBufferLength = GetCurrentDirectory(0, NULL);
    if (nBufferLength == 0) {
        stitch_log(STITCH_ERROR, "could not get current directory: %s", stitch_win32_error_message(GetLastError()));
        return NULL;
    }

    char *buffer = (char*) stitch_temp_alloc(nBufferLength);
    if (GetCurrentDirectory(nBufferLength, buffer) == 0) {
        stitch_log(STITCH_ERROR, "could not get current directory: %s", stitch_win32_error_message(GetLastError()));
        return NULL;
    }

    return buffer;
#else
    char *buffer = (char*) stitch_temp_alloc(PATH_MAX);
    if (getcwd(buffer, PATH_MAX) == NULL) {
        stitch_log(STITCH_ERROR, "could not get current directory: %s", strerror(errno));
        return NULL;
    }

    return buffer;
#endif // _WIN32
}

bool stitch_set_current_dir(const char *path)
{
#ifdef _WIN32
    if (!SetCurrentDirectory(path)) {
        stitch_log(STITCH_ERROR, "could not set current directory to %s: %s", path, stitch_win32_error_message(GetLastError()));
        return false;
    }
    return true;
#else
    if (chdir(path) < 0) {
        stitch_log(STITCH_ERROR, "could not set current directory to %s: %s", path, strerror(errno));
        return false;
    }
    return true;
#endif // _WIN32
}

// minirent.h SOURCE BEGIN ////////////////////////////////////////
#ifdef _WIN32
struct DIR
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

DIR *opendir(const char *dirpath)
{
    STITCH_ASSERT(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = (DIR*)STITCH_REALLOC(NULL, sizeof(DIR));
    memset(dir, 0, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        // TODO: opendir should set errno accordingly on FindFirstFile fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if (dir) {
        STITCH_FREE(dir);
    }

    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    STITCH_ASSERT(dirp);

    if (dirp->dirent == NULL) {
        dirp->dirent = (struct dirent*)STITCH_REALLOC(NULL, sizeof(struct dirent));
        memset(dirp->dirent, 0, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                // TODO: readdir should set errno accordingly on FindNextFile fail
                // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

int closedir(DIR *dirp)
{
    STITCH_ASSERT(dirp);

    if(!FindClose(dirp->hFind)) {
        // TODO: closedir should set errno accordingly on FindClose fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        return -1;
    }

    if (dirp->dirent) {
        STITCH_FREE(dirp->dirent);
    }
    STITCH_FREE(dirp);

    return 0;
}
#endif // _WIN32
// minirent.h SOURCE END ////////////////////////////////////////

#endif // STITCH_IMPLEMENTATION

#ifndef STITCH_STRIP_PREFIX_GUARD_
#define STITCH_STRIP_PREFIX_GUARD_
    // NOTE: The name stripping should be part of the header so it's not accidentally included
    // several times. At the same time, it should be at the end of the file so to not create any
    // potential conflicts in the STITCH_IMPLEMENTATION. The header obviously cannot be at the end
    // of the file because STITCH_IMPLEMENTATION needs the forward declarations from there. So the
    // solution is to split the header into two parts where the name stripping part is at the
    // end of the file after the STITCH_IMPLEMENTATION.
    #ifdef STITCH_STRIP_PREFIX
        #define TODO STITCH_TODO
        #define UNREACHABLE STITCH_UNREACHABLE
        #define UNUSED STITCH_UNUSED
        #define ARRAY_LEN STITCH_ARRAY_LEN
        #define ARRAY_GET STITCH_ARRAY_GET
        #define INFO STITCH_INFO
        #define WARNING STITCH_WARNING
        #define ERROR STITCH_ERROR
        #define NO_LOGS STITCH_NO_LOGS
        #define Log_Level Stitch_Log_Level
        #define minimal_log_level stitch_minimal_log_level
        // NOTE: Name log is already defined in math.h and historically always was the natural logarithmic function.
        // So there should be no reason to strip the `stitch_` prefix in this specific case.
        // #define log stitch_log
        #define shift stitch_shift
        #define shift_args stitch_shift_args
        #define File_Paths Stitch_File_Paths
        #define FILE_REGULAR STITCH_FILE_REGULAR
        #define FILE_DIRECTORY STITCH_FILE_DIRECTORY
        #define FILE_SYMLINK STITCH_FILE_SYMLINK
        #define FILE_OTHER STITCH_FILE_OTHER
        #define File_Type Stitch_File_Type
        #define mkdir_if_not_exists stitch_mkdir_if_not_exists
        #define copy_file stitch_copy_file
        #define copy_directory_recursively stitch_copy_directory_recursively
        #define read_entire_dir stitch_read_entire_dir
        #define write_entire_file stitch_write_entire_file
        #define get_file_type stitch_get_file_type
        #define delete_file stitch_delete_file
        #define return_defer stitch_return_defer
        #define da_append stitch_da_append
        #define da_free stitch_da_free
        #define da_append_many stitch_da_append_many
        #define da_resize stitch_da_resize
        #define da_reserve stitch_da_reserve
        #define da_last stitch_da_last
        #define da_remove_unordered stitch_da_remove_unordered
        #define String_Builder Stitch_String_Builder
        #define read_entire_file stitch_read_entire_file
        #define sb_appendf stitch_sb_appendf
        #define sb_append_buf stitch_sb_append_buf
        #define sb_append_cstr stitch_sb_append_cstr
        #define sb_append_null stitch_sb_append_null
        #define sb_free stitch_sb_free
        #define Proc Stitch_Proc
        #define INVALID_PROC STITCH_INVALID_PROC
        #define Fd Stitch_Fd
        #define INVALID_FD STITCH_INVALID_FD
        #define fd_open_for_read stitch_fd_open_for_read
        #define fd_open_for_write stitch_fd_open_for_write
        #define fd_close stitch_fd_close
        #define Procs Stitch_Procs
        #define procs_wait stitch_procs_wait
        #define procs_wait_and_reset stitch_procs_wait_and_reset
        #define proc_wait stitch_proc_wait
        #define Cmd Stitch_Cmd
        #define Cmd_Redirect Stitch_Cmd_Redirect
        #define cmd_render stitch_cmd_render
        #define cmd_append stitch_cmd_append
        #define cmd_extend stitch_cmd_extend
        #define cmd_free stitch_cmd_free
        #define cmd_run_async stitch_cmd_run_async
        #define cmd_run_async_and_reset stitch_cmd_run_async_and_reset
        #define cmd_run_async_redirect stitch_cmd_run_async_redirect
        #define cmd_run_async_redirect_and_reset stitch_cmd_run_async_redirect_and_reset
        #define cmd_run_sync stitch_cmd_run_sync
        #define cmd_run_sync_and_reset stitch_cmd_run_sync_and_reset
        #define cmd_run_sync_redirect stitch_cmd_run_sync_redirect
        #define cmd_run_sync_redirect_and_reset stitch_cmd_run_sync_redirect_and_reset
        #define temp_strdup stitch_temp_strdup
        #define temp_alloc stitch_temp_alloc
        #define temp_sprintf stitch_temp_sprintf
        #define temp_reset stitch_temp_reset
        #define temp_save stitch_temp_save
        #define temp_rewind stitch_temp_rewind
        #define path_name stitch_path_name
        #define rename stitch_rename
        #define needs_rebuild stitch_needs_rebuild
        #define needs_rebuild1 stitch_needs_rebuild1
        #define file_exists stitch_file_exists
        #define get_current_dir_temp stitch_get_current_dir_temp
        #define set_current_dir stitch_set_current_dir
        #define String_View Stitch_String_View
        #define temp_sv_to_cstr stitch_temp_sv_to_cstr
        #define sv_chop_by_delim stitch_sv_chop_by_delim
        #define sv_chop_left stitch_sv_chop_left
        #define sv_trim stitch_sv_trim
        #define sv_trim_left stitch_sv_trim_left
        #define sv_trim_right stitch_sv_trim_right
        #define sv_eq stitch_sv_eq
        #define sv_starts_with stitch_sv_starts_with
        #define sv_end_with stitch_sv_end_with
        #define sv_from_cstr stitch_sv_from_cstr
        #define sv_from_parts stitch_sv_from_parts
        #define sb_to_sv stitch_sb_to_sv
        #define win32_error_message stitch_win32_error_message
    #endif // STITCH_STRIP_PREFIX
#endif // STITCH_STRIP_PREFIX_GUARD_

// Random comment
