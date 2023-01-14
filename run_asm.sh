# make run
# arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -o debug.bin debug.s sysyruntimelibrary/libsysy.a
# qemu-arm -L /usr/arm-linux-gnueabihf debug.bin
# echo -e "\n"$?
clang -o debug.bin test.ll sysyruntimelibrary/sylib.c
test.bin
echo -e "\n"$?