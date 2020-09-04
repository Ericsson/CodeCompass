There are several environment variable which the logger uses (and you should know about):

# `CC_LOGGER_GCC_LIKE`
A colon separated list of commands that should be logged as gcc like command. If a list elem is a substring of an executed program then it will be logged. For example if gcc is in the list then gcc, gcc-4.7, x8_64-gcc-linux, etc... command will be also logged. This variable is set by default. For example (the default, set by setldlogenv.sh):
```bash
export CC_LOGGER_GCC_LIKE="gcc:g++:clang"
```

# `CC_LOGGER_JAVAC_LIKE`
The same as CC_LOGGER_GCC_LIKE but for logging java build process. This variable is set by default. For example (the default, set by setldlogenv.sh):
```bash
export CC_LOGGER_JAVAC_LIKE="javac"
```

# `CC_LOGGER_NO_DEF_DIRS`
If this environment variable is set to a any value, then the logger will not try to detect the default include paths for a gcc like command. This variable is not set by default. So if you want to turn off the default include path detection then type the following bash command:
```bash
export CC_LOGGER_NO_DEF_DIRS=1
```

You should set the these parameters before parsing.

# Parsing Java projects
If you build a Java project (for example: ant), you have to ensure that, the external javac will be invoked. You have to add the following attributes to the ant XML file:
```
fork="yes"
executable="javac"
```

Changes needed for Maven projects
```
<project>
  [...]
  <build>
    [...]
    <plugins>
      <plugin>
        [...]
        <configuration>
          [...]
          <fork>true</fork>
          <executable>javac</executable>
        </configuration>
      </plugin>
    </plugins>
    [...]
  </build>
  [...]
</project>
```
