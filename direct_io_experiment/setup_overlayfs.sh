#!/usr/bin/env bash
rm -r -f lower upper work merged
mkdir lower upper work merged
truncate -s 1g lower/file-1g
sudo mount -t overlay overlay -o lowerdir=lower,upperdir=upper,workdir=work merged

cp mmap_exec lower
