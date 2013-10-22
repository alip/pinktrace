#!/bin/sh

test -n "$GIT" || GIT=git

"$GIT" rev-parse --verify --short HEAD >/dev/null 2>&1
if test $? -ne 0; then
	echo "no git"
	exit 0
fi

"$GIT" log --pretty="format:%ci, commit:%h by %aN <%aE>" -1 -- "$1"
