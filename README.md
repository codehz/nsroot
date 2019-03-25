# nsROOT

<pre>
┌┐┌┌─┐╦═╗╔═╗╔═╗╔╦╗
│││└─┐╠╦╝║ ║║ ║ ║ 
┘└┘└─┘╩╚═╚═╝╚═╝ ╩ 
</pre>

`chroot`, `mount --bind` without privilege

## Installation

Options:
1. Use github release asset (x86_64, built with musl-gcc)
2. Compile from source
   - Prerequisites
     - `git`
     - `cmake`
     - `make` or `ninja`
   - Steps
      ```sh
      git clone https://github.com/CodeHz/nsroot.git
      cd nsroot
      mkdir build && cd build
      cmake .. && cmake --build . && sudo cmake --build . --target install
      ```

## Usage

```
nsroot [-x] [-X] [-p] [-f] [-r root] [-b dir[:target]] [-t target[:options]] [-c target] [-w workdir] /path/to/program [...args]
```

- `-x`<br>Setup user & mount namespace.
- `-X`<br>Setup mount namespace.
- `-p`<br>Mount proc filesystem.
- `-f`<br>Fork before exec.
- `-r path`<br>Use *path* as the new guest root file-system, default is `/`.
- `-b path`<br>Make the content of *path* accessible in the guest rootfs.
- `-t path`<br>Create tmpfs on *path* in guest rootfs.
- `-c path`<br>Create folder structure in guest rootfs.
- `-w path`<br>Set the initial working directory to *path*.
- `-a appname`<br>Set the initial process name.