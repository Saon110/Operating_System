#!usr/bin/bash

# echo -n "Enter a number: "
# read number1
# echo -n "Enter another number: "
# read number2

# if [ $number1 -gt $number2 ]; then
#     echo "1st number is greter"
# fi

# grade="A+"

# case $grade in
#     "A-") echo "Bad" ;;
#     "A") echo "Good" ;;
#     "A+") echo "Best" ;;
# esac


echo "the number of args is $#"
a=1
for i in $*
do
    echo "The $a No arg is $i"
    a=`expr $a + 1`
done