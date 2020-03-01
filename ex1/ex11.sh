# Narkis Kremizi Shallev 205832447
#!/bin/bash
# $1 = file, $2 = word
# prints to the screen only the lines contains the word + the line's number 
grep -n -w $2 $1 | tr ':' ' '