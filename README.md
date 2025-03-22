# stitch.h - A Minimalist Build System in a Single Header

**Stitch** is my take on a super lightweight build system that needs nothing but a C compiler. . The name **Stitch** reflects how this tool "stitches" together your build process without needing any external build tools.

## The NoBuild Idea

Why deal with `make`, `cmake`, or a million different shell scripts? With Stitch, all you need is a C compiler. You use it to bootstrap your build system, and then let the build system handle everything else. No shell scripts, no weird dependencies—just pure C.

## ⚠️ Experimental Project Alert

This is a personal experiment, and I'm still figuring out if it's a *good* idea. I'm actively using Stitch in my own projects, and so far, it works well for me. But if you have a giant project with tons of dependencies, this probably isn't what you're looking for. (Although if you're deep into CMake hell, you might have bigger problems than a build system .)

## Why Use Stitch?

✔ **Ultra-portable** – If your system has a C compiler, you can build your project. Works on Linux, macOS, Windows, FreeBSD, etc.

✔ **Same language for dev + build** – Your build system is just another part of your C codebase. You can even reuse project code inside your build scripts.

✔ **No external dependencies** – No `make`, no shell scripting, no batch files. Just drop in `stitch.h` and go.

✔ **More excuses to write C** – If you're a C nerd like me, this is a feature, not a bug.

## Why *Not* Use Stitch?

❌ **You need to be comfortable with C** – Stitch is basically writing shell scripts in C. If that sounds painful, this isn't for you.

❌ **Not meant for huge projects** – If you're managing dozens of dependencies with CMake, Stitch probably won't help.

❌ **You’ll be writing more C** – Again, this could be a pro or a con, depending on how you feel about C.

## Why Call It "Stitch" When It’s Clearly a Build System?

You know those trends like *NoSQL*, *No-Code*, and *Serverless* that claim to eliminate something but really just reinvent it? Yeah, this is the same idea. Traditional build systems annoyed me, so I decided to "stitch" together my own solution.

## Getting Started

You only need **one file**: [`stitch.h`](https://raw.githubusercontent.com/Unintellectual/stitch.h/refs/heads/main/stitch.h). Just copy-paste it into your project and start using it.

### Example

Here's a basic setup:

```c
// stitch.c
#define STITCH_IMPLEMENTATION
#include "stitch.h"

int main(int argc, char **argv)
{
    STITCH_GO_REBUILD_URSELF(argc, argv);
    Stitch_Cmd cmd = {0};
    stitch_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-o", "main", "main.c");
    if (!stitch_cmd_run_sync(cmd)) return 1;
    return 0;
}
```

Compile and run it like this:

```sh
$ cc -o stitch stitch.c
$ ./stitch
```

Thanks to the `STITCH_GO_REBUILD_URSELF` macro, **Stitch** will automatically rebuild itself if `stitch.c` changes. Check out [`./tests/`](./tests/) and [`./how_to/`](./how_to/) for more examples.

---

Hope you find this interesting! If you try **Stitch**, let me know how it works for you. 🚀

