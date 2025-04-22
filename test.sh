#!/bin/sh
set -efu

if ! [ -x sponge ]; then
	echo 'sponge executable not found' >&2
	exit 1
fi

# ensure output is larger than 8192 bytes
alias hello='yes hello world | head -n 1000'

touch file_a.txt file_b.txt
trap 'rm file_a.txt file_b.txt' EXIT

tests=''

tests="$tests test_file"
test_file() {
	hello | ./sponge file_a.txt
	local actual="$(cat file_a.txt)"
	local expected="$(hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_multiple_files"
test_multiple_files() {
	hello | ./sponge file_a.txt file_b.txt
	local actual_a="$(cat file_a.txt)"
	local actual_b="$(cat file_b.txt)"
	local expected="$(hello)"
	[ "x$actual_a" = "x$expected" ] && [ "x$actual_b" = "x$expected" ]
	return $?
}

tests="$tests test_append_file"
test_append_file() {
	hello > file_a.txt
	hello | ./sponge -a file_a.txt
	local actual="$(cat file_a.txt)"
	local expected="$(hello; hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_append_multiple_files"
test_append_multiple_files() {
	hello | tee file_a.txt file_b.txt > /dev/null
	hello | ./sponge -a file_a.txt file_b.txt
	local actual_a="$(cat file_a.txt)"
	local actual_b="$(cat file_b.txt)"
	local expected="$(hello; hello)"
	[ "x$actual_a" = "x$expected" ] && [ "x$actual_b" = "x$expected" ]
	return $?
}

tests="$tests test_stdout"
test_stdout() {
	local actual="$(hello | ./sponge)"
	local expected="$(hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_stdout_dash"
test_stdout_dash() {
	local actual="$(hello | ./sponge -)"
	local expected="$(hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_files_and_stdout"
test_files_and_stdout() {
	local actual_stdout="$(hello | ./sponge file_a.txt - file_b.txt)"
	local actual_a="$(cat file_a.txt)"
	local actual_b="$(cat file_b.txt)"
	local expected="$(hello)"
	[ "x$actual_stdout" = "x$expected" ] && [ "x$actual_a" = "x$expected" ] && [ "x$actual_b" = "x$expected" ]
	return $?
}

tests="$tests test_sponge"
test_sponge() {
	hello > file_a.txt
	cat file_a.txt | ./sponge file_a.txt
	local actual="$(cat file_a.txt)"
	local expected="$(hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_sponge_multiple_files"
test_sponge_multiple_files() {
	hello | tee file_a.txt file_b.txt > /dev/null
	cat file_a.txt file_b.txt | ./sponge file_a.txt file_b.txt
	local actual_a="$(cat file_a.txt)"
	local actual_b="$(cat file_b.txt)"
	local expected="$(hello; hello)"
	[ "x$actual_a" = "x$expected" ] && [ "x$actual_b" = "x$expected" ]
	return $?
}

tests="$tests test_sponge_append"
test_sponge_append() {
	hello > file_a.txt
	cat file_a.txt | ./sponge -a file_a.txt
	local actual="$(cat file_a.txt)"
	local expected="$(hello; hello)"
	[ "x$actual" = "x$expected" ]
	return $?
}

tests="$tests test_sponge_append_multiple_files"
test_sponge_append_multiple_files() {
	hello | tee file_a.txt file_b.txt > /dev/null
	cat file_a.txt file_b.txt | ./sponge -a file_a.txt file_b.txt
	local actual_a="$(cat file_a.txt)"
	local actual_b="$(cat file_b.txt)"
	local expected="$(hello; hello; hello)"
	[ "x$actual_a" = "x$expected" ] && [ "x$actual_b" = "x$expected" ]
	return $?
}

n=0
fails=0
for t in $tests; do
	: $((n += 1))
	if ! eval "$t"; then
		echo "$t failed"
		: $((fails += 1))
	fi
done
if [ "$fails" -eq 0 ]; then
	echo 'all tests passed'
else
	echo "$fails/$n tests failed"
fi
