// This is your build script. You only need to "bootstrap" it once with `cc -o stitch stitch.c` (you can
// call it whatever actually) or `cl stitch.c` on MSVC and thanks to STITCH_GO_REBUILD_URSELF (see below).
// After that every time you run the `stitch` executable if it detects that you modifed stitch.c it will
// rebuild itself automatically

// stitch.h is an stb-style library https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
// What that means is that it's a single file that acts both like .c and .h files, but by default
// when you include it, it acts only as .h. To make it include implementations of the functions
// you must define STITCH_IMPLEMENTATION macro. This is done to give you full control over where
// the implementations go.
#define STITCH_IMPLEMENTATION

// Always keep a copy of stitch.h in your repo. One of my main pet peeves with build systems like CMake
// and Autotools is that the codebases that use them naturally rot. That is if you do not actively update
// your build scripts, they may not work with the latest version of the build tools. Here we basically
// include the entirety of the source code of the tool along with the code base. It will never get
// outdated.
//
// (In these examples we actually symlinking stitch.h, but this is to keep stitch.h-s synced among all the
// examples)
#include "stitch.h"

// TODO: add more comments in here

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

int main(int argc, char **argv)
{
    STITCH_GO_REBUILD_URSELF(argc, argv);

    Stitch_Cmd cmd = {0};

    if (!stitch_mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    stitch_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", BUILD_FOLDER"main", SRC_FOLDER"main.c");
    if (!stitch_cmd_run_sync(cmd)) return 1;

    return 0;
}
