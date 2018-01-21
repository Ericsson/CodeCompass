# This file is an adaptation layer for CodeCompass framework.
# It is sourced to the bash scripts of the toolset and provides an API.
# When you plan to adapt your project to the framework, these functions
# should be ported.
# The toolset binaries and this file should be placed in the same directory.
# The project specific helper scripts can be installed in other place.

build_directory="/tmp/build"
compile_commands_json="${build_directory}/compile_commands.json"

# Installing project specific things.
# Parameters:
# 1 - Directory where the binaries of the toolset was installed.
#
# 2 - Directory where the configuration files are placed.
#
# 3 - Directory where the project specific files are placed.
#
# Xerces:
# The build directory is set up on the host, that is mounted by the Docker.
#
install_project() {
    local scriptdir="${1}"
    local configdir="${2}" # Unused
    local workspacedir="${3}"

    echo ${workspacedir}
    ls -la ${workspacedir}
    if [[ ! -r  "${workspacedir}/xerces-c.spec" ]]; then
        echo "Xerces source could not be found." >&2
        exit 1
    fi
    mkdir -p "${build_directory}"
}

# Query the file name of the "JSON compilation database".
# See http://clang.llvm.org/docs/JSONCompilationDatabase.html
#
# Parameters:
# 1 - Directory where the database should be created.
#
# Return:
# Full name of the compilation database file.
#
get_compilation_database() {
    echo "${compile_commands_json}"
}

# Query the logging controller flag. Returns true (0) if the project requires
# logging phase (pre-build) to produce "JSON compilation database".
#
# Return:
# 0 if the project needs logging phase before parsing phase to produce compilation
# database on demand.
# 1 - When there is  compilation database that reachable by the framework at the
# moment of the parsing phase.
#
# Sometimes the logging phase is not necessary, for example the project has got
# a compilation database.
#
# Xerces:
#  Logging is necessary.
#
is_logging_necessary() {
    return 0
}

# Filter the file name of the "JSON compilation database".
# See http://clang.llvm.org/docs/JSONCompilationDatabase.html
# Generates a new filtered compilation database file.
# Parameters:
# 1 - Directory where the binaries of the toolset was installed.
# 2 - Directory where the database should be created.
#
# Return:
# Full name of the pre-processed/filtered compilation database file.
#
# Sometimes the compilation database contains unnecessary compile commands or
# requires modifications that allows the clang compiler to run well.
# This hook allows project specific editing before parsing phase.
#
filter_compilation_database() {
    local scriptdir="${1}"
    local compilation_database=$(readlink -e "${2}")
    local dir_name=$(dirname "${compilation_database}")
    local filtered_name="${dir_name}/filtered.$(basename ${compilation_database})"
    "${scriptdir}/filterbuildcmds.py" --input "${compilation_database}" \
        --output "${filtered_name}"
    echo "${filtered_name}"
}

# Build the project.
#
# Parameters:
# 1 - Directory where the binaries of the toolset was installed.
#
# 2 - Directory where the configuration files are placed.
#
# 3 - Directory where the project specific files are placed.
#

# This function will be called during logging phase. The CodeCompass logger
# generates compilation database based on calls of compiler that this function
# causes.
#
build_the_project() {
    local scriptdir="${1}" # Unused
    local configdir="${2}" # Unused
    local workspacedir="${3}" # unused

    pushd "${build_directory}"
    # Generate makefiles.
    cmake ${workspacedir}
    # Build the project
    make -j 4
    popd
}

# Queries directories where the source of the project placed.
#
# Parameters:
# 1 - Directory where the binaries of the toolset was installed.
#
# 2 - Directory where the configuration files are placed.
#
# 3 - Directory where the project specific files are placed.
#
# These directories will be scanned by the plugins of the CodeCompass.
# For example the source code indexer plugin. That provides search by file
# name.
#
# Return:
# Newline separated string of directory names.
#
get_source_dir_array() {
    local scriptdir="${1}" # Unused
    local configdir="${2}" # Unused
    local workspacedir="${3}"

    echo "${workspacedir}"
}

# Queries file name pattern of the observed compiler.
#
# Return:
# Colon separated program names as described in the CodeCompass documentation.
#
get_compiler_pattern() {
    echo "cc:c++"
}

