#!/bin/bash

INIT_COMMIT=$(git log --reverse --pretty=format:"%h" | head -n 1)
IGNORES=FILE_PATH_CHANGES,SPDX_LICENSE_TAG

HW5_IGNORES=$IGNORES,EXPORT_SYMBOL,ENOSYS,AVOID_EXTERNS
git diff $INIT_COMMIT | linux/scripts/checkpatch.pl --ignore $HW5_IGNORES

# ENOSYS: Looks like ENOSYS is often misused in the kernel so checkpatch always
#         warns on seeing ENOSYS. Our use is correct, though!
#         (Nonexistent syscall was called)
# EXPORT_SYMBOL: Buggy for our use-case, let's just ignore this.
# AVOID_EXTERNS: Checkpatch will complain if you extern in a .c file instead of
#                a .h file.
