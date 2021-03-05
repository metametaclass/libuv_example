Basic [libuv](https://github.com/joyent/libuv/) example.

Build with msys2/mingw64:

```
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-libuv
git clone ...
cd libuv_example
make all
```

Run `build_Windows/libuv_test.exe` from cmd.exe. Msys2 shell doesn`t work, because of [bad non-tty stdin descriptor](https://github.com/TypeStrong/ts-node/issues/1081)
