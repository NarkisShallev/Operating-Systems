# Narkis Kremizi Shallev 205832447
#!/bin/bash
# $1 = full name, $2 = bank file
totalBalance=0
# Prints all the lines in a bank file that contains the customer's name
grep -w "$1" $2
# Prints the total balance in the customer's account
grep -w "$1" $2 | awk -v x=$totalBalance '{x+=$3;} END {print "Total balance: " x;}'