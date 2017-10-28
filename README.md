Single commands work as intended
Pipes are not working, single pipe was working
However when the program was changed to handle multiple
pipes it broke
Redirection is misbehaving it worked for in or out but not both
before the inclusion of a double pipe function
The out direction operator if used redirects print statements inside
the cmdHandler and doInsertion function to the output
so if you run
< echo "hello"  > test.txt >  puts print statement included in the file for testing will print to the file but not the string "hello" 
so the file descriptor is setup correctly but something is happening in cmdHandler.
There are print statements printing out variouse variable in cmdHandler to try and track where the problems are.
