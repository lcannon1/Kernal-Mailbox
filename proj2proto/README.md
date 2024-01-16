# list.h for userspace
This is the kernel's list.h with some modifications so it works in userspace.
All you need is `list.h` and `list_hacks.h`. This repository also includes some
examples on how to use the list, in case you need them.

To use the list you need to be compiling with the gnu90 standard. You must also
not be using the -pendantic flag for gcc.

# Building the examples
Just `cd` to the examples directory and run `make`. This will create an
executable called `list_examples` in the examples directory.

# NOTE
This code is **NOT** provided to you by your TAs. You **MUST** reference it and
cite it if you use it. You are not allowed to copy/paste this code into your
project(with the exception for list.h and list_hacks.h). Doing so is plagiarism.