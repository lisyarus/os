#!/bin/bash

test_symlink_script="/tmp/test-symlink.sh"

echo -e '#!/bin/bash\nif [[ -h $1 && ! -e $1 ]]; then exit 0; else exit 1; fi' > "$test_symlink_script"

chmod +x "$test_symlink_script"

find "$1" -print0 | ./filter -z -- "$test_symlink_script" | xargs -0 echo

rm -f "$test_symlink_script"

exit 0
