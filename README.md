# Sponge

Soak up standard input and write to files.
Inspired by the `sponge` utility from [moreutils](https://joeyh.name/code/moreutils/).

## Installation

```sh
$ make
$ sudo make install
```

## Usage

```sh
# sponge, then write to same file
$ cat file | sponge file

# write to multiple files
$ cat file_1 | sponge file_1 file_2

# append to file(s) with the -a flag
$ cat file_1 | sponge -a file_1 file_2

# tee by specifying - as an output file
$ cat file | sponge file -
```
