#!/bin/sh

path="${1}"
if ! test -f "${path}"
then
    echo "usage ${0##*/} <file>" >&2
    exit 1
fi

topdir=$(git rev-parse --show-toplevel)
if test -z "${topdir}"
then
    echo "no git?!" >&2
    exit 1
fi

exec "${topdir}"/misc/checkpatch.pl --no-tree --file "${path}"

#exec find "${path}" \
#    '(' -name '*.[hc]' -o -name '*.h.in' ')' \
#    -a '(' -not -name about.h ')' \
#    -a '(' -not -name system.h ')' \
#    -a '(' -not -name wildmatch.c ')' \
#    -a '(' -not -path '*/pinktrace/linux/*' ')' \
#    -exec uncrustify -c "${topdir}"/misc/linux.cfg --no-backup -l C '{}' \;
