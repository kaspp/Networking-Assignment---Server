README

USAGE: 
	$ make 
	to compile file. 
	Expected to see the below after commenting:
		gcc -W -Wall -c -pthread main.c
		gcc -W -Wall -o server main.o

if using default port (PORT 80)
	<Example>
	$ make start

	log onto browser and type in address	
	http://172.20.237.45:80

else using other port
	$ make <specify port no.> 

	<Example> 
	$ make 8000

	log onto browser and type in address
	http://172.20.237.45:8000
