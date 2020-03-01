# Narkis Kremizi Shallev 205832447
#!/bin/bash
# $1 = directory
# save the working directory
previousDirectory=$(pwd)
# go to the received directory
cd $1
# prints in lexicographic order the names of the files that end with .txt and "is a file"
ls | find -mindepth 1 -maxdepth 1 -type f -name "*.txt"| cut -c3- | LC_ALL=C sort | sed 's/$/ is a file/'
# prints in lexicographic order the names of the folders and "is a directory"
ls | find -mindepth 1 -maxdepth 1 -type d | cut -c3- | LC_ALL=C sort | sed 's/$/ is a directory/'
# go back to the working directory
cd $previousDirectory