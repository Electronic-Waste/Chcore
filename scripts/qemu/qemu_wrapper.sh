#!/bin/bash

set -e

# return true if @v1 <= @v2
verlte() {
    [ "$1" = "$(echo -e "$1\n$2" | sort -V | head -n 1)" ]
}

verlt() {
    [ "$1" = "$2" ] && return 1 || verlte $1 $2
}

qemu=$1
shift
qemu_options=$@
qemu_version_str=$($qemu --version | head -n 1)
export IFS=' '
flag="false"
qemu_version=${qemu_version_str}
for str in ${qemu_version_str};
do
    if [[ "${str}" == "version" ]]; then
        flag="true"
    elif [[ ${flag} == "true" ]]; then
        qemu_version=${str}
        break
    fi
done
unset IFS

if [[ "$qemu" == *"qemu-system-aarch64"* ]]; then
    if verlt $qemu_version 6.2.0; then
        # in qemu < 6.2.0, machine type = raspi3
        # in qemu >= 6.2.0, machine type = raspi3b
        qemu_options=$(echo $qemu_options | sed 's/-machine[ \t]\{1,\}raspi3b/-machine raspi3/g')
    fi
fi

$qemu $qemu_options
