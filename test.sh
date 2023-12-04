test_folders=$(ls tests)
success_tests=0
total_tests=0
for test_folder in $test_folders
do
    echo "Running $test_folder tests"

    test_files=$(ls tests/$test_folder | grep .bc)
    mkdir -p logs/$test_folder
    for test_file in $test_files
    do
        total_tests=$(( $total_tests + 1 ))
        filename=$( basename $test_file )
        log_filename="${filename/bc/out}"
        input_filename="${filename/bc/input}"
        orig_filename="${filename/bc/log}"

        touch "logs/$test_folder/$log_filename"
        ./byterun/byterun "tests/$test_folder/$test_file" < "tests/$test_folder/$input_filename" > "logs/$test_folder/$log_filename" &> "logs/$test_folder/$log_filename"
        byterun_exit_code=$?
        cmp -s "tests/$test_folder/orig/$orig_filename" "logs/$test_folder/$log_filename"
        cmp_exit_code=$?


        if [[ $byterun_exit_code != 0 || $cmp_exit_code != 0 ]]; then
            echo "Test $filename failed"
        else
            success_tests=$(( $success_tests + 1 ))
        fi
    done
done

echo "Test success: $success_tests/$total_tests"