# libuv_example

Basic [libuv](https://github.com/joyent/libuv/) example.

## Build with msys2/mingw64:

```
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-libuv
git clone ...
cd libuv_example
make all
```

Run `build_Windows/libuv_test.exe` from cmd.exe. Msys2 shell doesn`t work, because of [bad non-tty stdin descriptor](https://github.com/TypeStrong/ts-node/issues/1081)


---

## Build and debug with gcc on ubuntu linux

```
# install libuv1 packages
sudo apt-get install libuv1 libuv1-dev libuv1-dbg
# clone project
git clone ...
cd libuv_example
# install libuv source code for gdb
sudo apt-get source libuv1
# build
make all
# run in gdb
gdb ./build_Linux/libuv_test
gdb> dir libuv1-1.8.0
gdb> b uv_loop_close
gdb> r
```

