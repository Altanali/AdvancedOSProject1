#!/usr/bin/env bash
sudo umount merged
rm -r -f lower upper work merged
mkdir lower upper work merged
truncate -s 1g lower/file-1g
sudo mount -t overlay overlay -o lowerdir=lower,upperdir=upper,workdir=work merged
../read_file lower/file-1g
cp mmap_exec lower
cp run_mmap.sh lower

docker run -it --mount type=bind,source="$(pwd)/merged",target=/usr/home --entrypoint bash --cpus="2" --memory="2g" ubuntu

