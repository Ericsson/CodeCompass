# Lazy Dog

1. Install dependencies (Ubuntu 64-bit)
    + Open a new terminal
    + ``` sudo apt-get install gcc-multilib g++-multilib python-dev zlib1g-dev libssl-dev subversion```

2. Build CodeCompass (For more information click [here](build_codecompass.md))
    + Create directory for CodeCompass
         ```    
         mkdir cc
         cd cc
         ```
3. Clone the CodeCompass from the repository
    + ```git clone https://github.com/Ericsson/CodeCompass.git```

7. Build the dependencies
    + ```CodeCompass/scripts/build_deps.sh```
	
8. Load the CodeCompass build environment
    + ```source CodeCompass-install/env.sh```

9. Build CodeCompass with pgsql
     + ```./autogen.sh ```
     + ```./configure --with-database=pgsql --prefix=/home/username/cc/CodeCompass-install/```

10. Rebuild CodeCompass
     + ```make -j4```

11. Update installation files
     + ```make install```

12. Init and start database
     + Open a new terminal
     + Load build environment - ```source CodeCompass-install/env.sh```
     + Go to directory, where CodeCompass is - ``` cd cc ```
     + Make a directory for database files - ``` mkdir database```
     + Go to database directory - ``` cd database ```
     + Init database - ``` initdb -D . -E "SQL_ASCII" ```
     + Start database - ``` postgres -D . -p 6250 ```

13. Parse a project (For more information click [here](parse_a_project.md))
     + Download tinyxml - http://sourceforge.net/projects/tinyxml/files/latest/download
     + Open a new terminal
     + Load build environment - ```source CodeCompass-install/env.sh```   
     + Make a directory for projects - ```mkdir cc/projects```   
     + Unzip tinyxml to cc/projects
     + Make a working directory - ``` mkdir cc/workdir```
     + Go to tinyxml directory - ```cd /home/username/cc/projects```
     + Parse tinyxml project - ```CodeCompass-install/bin/CodeCompass parse -w /home/username/cc/workdir -n tinyxml  -b "g++ *.cpp -std=c++11" --dbuser username --dbport 6250```

14. Start webserver (For more information click [here](start_codecompass_webserver.md))
     + Open a new terminal
     + Load build environment - ```source CodeCompass-install/env.sh```   
     + Start server - ```CodeCompass-install/bin/CodeCompass server -w /home/username/cc/workdir -p 6251 --dbport 6250 --dbuser username```

15. Check CodeCompass
     + Start a web browser
     + Connect to the server you started in (```localhost:6251```)
