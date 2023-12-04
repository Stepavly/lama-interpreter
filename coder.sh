cd tests/deep-expressions
test_files=$(ls | grep .lama)
for test_file in $test_files
do
    lamac -b $test_file
done