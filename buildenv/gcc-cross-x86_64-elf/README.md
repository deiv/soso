This code was shamelessly taken from https://github.com/randomdude/gcc-cross-x86_64-elf.
Debian, Binutils and Gcc versions were updated in the process.

Anyway, this will build an x86_64-elf-gcc toolchain. I use it to build code which runs without an OS present (ie, GRUB loads it).

The resulting image is of an Debian image with the built toolchain installed and ready to go.

To use it, you can simply build and run this container, and then 'docker exec gcc ...' to compile.
Or, you can build your project in a container based on this one. For an example, see the 'btstestbench' project, which does this:
```
FROM deiv/gcc-cross-x86_64-elf

RUN apt-get update 
RUN apt-get upgrade -y
RUN apt-get install -y grub-common

COPY ./ /root/

WORKDIR /root/
ENTRYPOINT build.sh # which invokes gcc/make for your own code
```
