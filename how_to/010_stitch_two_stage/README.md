# Two Stage Build

Quite often when your project grows big enough you end up wanting to configure the build of your application: different sets of enabled features, compilation flags, etc. The recommended approach to this in stitch is to setup Two Stage Build system. That is [stitch.c](./stitch.c) does not do any actual work except generating initial `./build/config.h` and building `./src_build/stitch_configed.c` (which `#include`-s the `./build/config.h` file) and then running it.

Exact details of the setup is up to your. Here we present just one way of doing it. In fact you have a freedom to do as many stages of your build as you want, analysing your environment in all sorts of different ways (you literally have a system programming language at your disposal). The whole point of stitch is that you bootstrap it ONLY with `cc -o stitch stitch.c` (no additional flags or actions should be required form the user) and the rest is taken care of by your C code.

## Quick Start

```console
$ cc -o stitch stitch.c
$ ./stitch
$ ./build/main
$ <edit ./build/config.h>
$ ./stitch
$ ./build/main
```
