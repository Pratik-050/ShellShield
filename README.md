# ShellShield

This project includes building a simple container from scratch.
It'll contain system,process and signals concepts. We'll use C++
to code the entire project. By the end of the 10 days you'll have
a strong understanding of how processes work behind the scenes and
the primary technology, NAMESPACES, that is used by Docker to
build containers.

# How to run

While running first time you need to install a root operating system to run inside the container.
Here we provided the download link for **alpine-miniroot-fs** or **ubuntu-rootfs**(A small OS generally used for testing purposes):

https://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-minirootfs-3.19.0-x86_64.tar.gz


http://cdimage.ubuntu.com/ubuntu-base/releases/22.04.2/release/ubuntu-base-22.04-base-amd64.tar.gz

make a root directory in the directory where you cloned the repo and unzip the tar file inside that using

    tar -xvzf alpine-minirootfs-3.19.0-x86_64.tar.gz
    or
    tar -xvzf ubuntu-base-22.04-base-amd64.tar.gz


use the makefile for automated compilation.

    make -s && sudo ./container
