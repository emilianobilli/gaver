#The MIT License (MIT)

#Copyright (c) 2013 Julien Andrieux <chilladx@gmail.com>

#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

# function to parse the ini style configuration file
config_parser () {
	local iniFile="$1";
	local tmpFile=$( mktemp /tmp/`basename $iniFile`.XXXXXX );
	local intLines;
	local binSED=$( which sed );

	# copy the ini file to the temporary location
	cp $iniFile $tmpFile;

	# remove tabs or spaces around the =
	$binSED -i -e 's/[ \t]*=[ \t]*/=/g' $tmpFile;

	# transform section labels into function declaration
	$binSED -i -e 's/\[\([A-Za-z0-9_]*\)\]/config.section.\1() \{/g' $tmpFile;
	$binSED -i -e 's/config\.section\./\}\'$'\nconfig\.section\./g' $tmpFile;

	# remove first line
	$binSED -i -e '1d' $tmpFile;

	# add the last brace
	echo -e "\n}" >> $tmpFile;

	# now load the file
	source $tmpFile;

	# clean up
	rm -f $tmpFile;
}
