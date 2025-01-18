#! /usr/bin/env bash
change_files=$(git diff --cached --name-only | grep -E '\.(c|h|cpp|hpp)$')
change_files="${change_files} $(git ls-files -mo --exclude-standard | grep -E '\.(c|h|cpp|hpp)$')"

if [[ -z "${change_files}" || "${change_files}" =~ ^[[:space:]]*$ ]]
then
    echo "no changed files to prep"
else
    if [[ ! -f "checkpatch.pl" ]]
    then
        wget https://raw.githubusercontent.com/intel/zephyr/main/scripts/checkpatch.pl
        chmod +x checkpatch.pl
    fi
    for f in $change_files; do
        echo "### run clang-format for ${f}"
        clang-format-20 -i ${f}
        echo "### run dos2unix for ${f}"
        dos2unix ${f}
        echo "### run checkpatch for\n${f}"
        ./checkpatch.pl --no-tree --file ${f}
        echo "---------------------"
    done
fi

