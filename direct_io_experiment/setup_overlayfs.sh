#!/usr/bin/env bash
sudo umount merged
rm -r -f lower upper work merged
mkdir lower upper work merged
truncate -s 1g lower/file-1g
sudo mount -t overlay overlay -o lowerdir=lower,upperdir=upper,workdir=work merged

../read_file lower/file-1g

cp direct_io lower
cp run_overlay_experiment.sh lower

docker run -it --mount type=bind,source="$(pwd)/merged",target=/usr/home \
 --entrypoint bash ubuntu

