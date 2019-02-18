Build instructions:

Note: Run all commands from the directory above
      Scripts are tested with cygwin

	  
	  
Prepare
=======
	update version.sh file with correct version information
	-> see also some instructions there
	copy/update the json.template file with the current released json files.
	
	  
Build Compiler package with new GCC (elf) compiler
==================================================
	run ./extras/make_mito_release.sh
	- files are collected from the Energia.nu web page with the source defined in version.sh 
      -> just used to calculate the sha256
	- dslite is assumed to be available on the web ready to use package
	- the json file is generated / updated / added with the new or changed version and file details
	- finally the files are stored in the build folder


