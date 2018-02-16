Installation of MORPH v1.0.6 on Linux (Ubuntu)
==============================================
1. Dependencies 
~~~~~~~~~~~~~~~
MORPH 1.0.6 depends on:
	- CMake
	- boost: filesystem iostreams system serialization
	- gsl
	- yamlcpp

Chances are you don't have the last two, on ubuntu (or other 
aptitude using platforms) you can easily install these:

	sudo apt-get install libgsl-dev
	sudo apt-get install libyaml-cpp-dev

2. Compilation
~~~~~~~~~~~~~~
Normally the following commands should suffice:

	cd build/release
	./cmake_
	make

Then run the `morph` executable and you should get
the following response:

--------------------------------------------------------------------------------------------------------------------
	USAGE: morphc path/to/config.yaml path/to/joblist.yaml path/to/output_directory top_k [--output-yaml]

	top_k = max number of candidate genes to save in outputted rankings
	--output-yaml = when specified, rankings are saved in yaml format, otherwise they are saved in plain text format

	RETURN CODES:
	  0: No error
	  1: Generic error
	  2: GOI contains gene with invalid name


	Exception: Invalid argument count
--------------------------------------------------------------------------------------------------------------------

Now you're ready for MORPHing around!



