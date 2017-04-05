This is the home of what's currently a proof-of-concept (p-o-c) experiment to create a module for Apache 2 Web Servers that serves content from ZIM files.

== Prerequisites ==

== Building the code ==
The build is currently performed by a crude, limited bash script `build.sh`

== Loading the new module ==
Run `sudo service apache restart`

== Known limitations ==
Here are our known limitations, some will be removed or addressed as we enhance the code.

- Hardcoded behaviour and ZIM file
- Not designed for multi-use or repeated-use
- No logging and no integration with Apache logging or error reporting
- No way to test the functionality except interactively and manually
