#!/bin/bash

# COLORS
# Color_Off   '\033[0m'
# Red         '\033[0;31m'
# Green       '\033[0;32m'
# Yellow       '\033[0;33m'

# shellcheck disable=SC2010
testCases=$(ls scanner| grep case_.)
numberOfCases=0
numberOfSuccess=0

for file in $testCases;
do
  name=$file
  result="result_${file:5}"

  # cat ../tests/test_file_01.txt | ./test-scanner
  #output=$(cat scanner/$file | ../src/test-scanner)
  cat "scanner/$file" | ../src/test-scanner | sed 's/\x1B\[[0-9;]\{1,\}[A-Za-z]//g' > "scanner/output_${file:5}"

  difference=$(diff "scanner/output_${file:5}" "scanner/$result")

  if [ $? -eq 1 ]; then
    echo -e "\033[0;31m ${name} \033[0m"
    echo "$difference"
  else
    echo -e "\033[0;32m ${name} \033[0m"
    ((numberOfSuccess=numberOfSuccess + 1))
  fi

  ((numberOfCases=numberOfCases + 1))

  echo -e "\033[0;33m-----------------------------------------------------------------------------\033[0;33m"
done

echo -e "\033[0;33m${numberOfSuccess} out of ${numberOfCases} tests were successful\033[0;32m"
