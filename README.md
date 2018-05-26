# file-system-using-Fuse
A FUSE filesystem is a program that listens on a socket for file operations to perform, and performs them.
To run gcc new.c -o new `pkg-config fuse --cflags --libs`
mkdir mount
./new -f mount
