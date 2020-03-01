# Narkis Kremizi Shallev 205832447
#!/bin/bash
# $1 = directory, $2 = file
# save the working directory
previousDirectory=$(pwd)
# go to the received directory
cd "$1"
# Searches for files with the same name in the received directory in a lexicographic order.
# If a file with the same name is found, its contents are printed. 
for item in $(ls | LC_ALL=C sort); do
	if [[ -d "$item" ]]; then
		cd "$item"
		for file in $(ls | LC_ALL=C sort); do
			if [[ ("$file" == "$2") && (-f "$file") ]]; then
				cat "$file"
			fi
		done
		# go back to the outer directory
		cd ..
	#Does the same but files in the resulting path.
	else
		if [[ "$item" == "$2" ]]; then
				cat "$item"
		fi
	fi
done
# go back to the working directory
cd $previousDirectory