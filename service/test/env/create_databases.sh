#!/bin/bash

# Service test environment setup script
#
# Parameters:
#   $1: porject parser binary
#   $2: source root directory (top_src_dir)
#   $3: test directory dirstamp
#
# Exports:
#   SERVICE_TEST_DB_DIR: Temporary directory for service tests

# check SERVICE_TEST_DB_DIR
if [ -z "$SERVICE_TEST_DB_DIR" ]; then
  echo "No SERVICE_TEST_DB_DIR!!!"
  exit -1
fi

#
# Global variables
#

# parser command
parserCommand=$1
# base directory
sourceRootDir=$2
# service tests source directory
serviceTestSrcDir="$2/service/test/sources"
# test dirstamp path
testDirstampPath=$3
# dirstamp
dirstamp="$SERVICE_TEST_DB_DIR/.dirstamp"

# Logs a debug message
function logD() {
  #echo "$@" 1>&2;
  echo "$@" 1>/dev/null;
}

# Deletes the previous temporary directory (if exists)
function cleanTestDir() {
  logD "Cleaning service test directory ($SERVICE_TEST_DB_DIR)"
  logD
  rm -fr $SERVICE_TEST_DB_DIR
}

# Parses a directory or an xml
#
# Parameter:
#   $1: database name (only for logging)
#   $2: database directory name (target under the test directory)
#   $3: database name (filename, eg. tiny.sqlite)
#   $4: project to parse (xml or directory relative to source root (top_src_dir))
function parseSource() {
  logD
  logD "Creating database $1 to $2"
  logD " project = $4"
  logD " database name = $3"

  dbDir="$SERVICE_TEST_DB_DIR/$2"
  mkdir -p "$dbDir/data"
  dbParserCommand="$parserCommand -d 'sqlite:database=$dbDir/$3' -a '$dbDir/data' -p '$4' -l debug &> '$dbDir/parselog'"

  mkdir -p "$dbDir"
  if [ $? -ne 0 ]; then
    echo "Failed to create $dbDir !!!"
    exit -1
  fi

  logD "Execute $dbParserCommand"
  eval $dbParserCommand
  parseResult=$?
  logD "Parse result: $dbParserCommand"
  logD

  if [ $parseResult -ne 0 ]; then
    echo "Failed to parse a project!"
    exit -1;
  fi
}

# Params same as parseSource
function parseJavaSourceDir {
  # cleanup class files
  find ${4} -name '*.class' -exec rm {} \;
  find ${4} -name '*.java' -exec javac {} \+

  parseSource "$@"
}

# Extracts a zip file and parses it
# Needed for Git projects
#
# Parameter:
#   $1-$3: same as parseSource
#   $4: zip file to extract before parsing
#   $5: location to extract
function parseZippedSource {
  mkdir -p $5
  unzip -q $4 -d $5

  parseSource $1 $2 $3 $5
}

function createDatabases() {
  mkdir -p "$SERVICE_TEST_DB_DIR"

  parseSource "Empty" "empty" "empty.sqlite" "$serviceTestSrcDir/empty"
  parseSource "Simple" "simple" "simple.sqlite" "$serviceTestSrcDir/simple"
  parseSource "CppTest" "cpptest" "cpptest.sqlite" "$serviceTestSrcDir/cpptest"
  parseSource "TinyXml" "tinyxml" "tinyxml.sqlite" "$serviceTestSrcDir/tinyxml"
  parseSource "SimpleXMLProj" "simple_xmlproj" "simplexml.sqlite" "$serviceTestSrcDir/simple_xmlproj/simple.xml"
  parseSource "pdgproj" "pdg" "pdg.sqlite" "$serviceTestSrcDir/pdg"
  parseSource "SearchMix" "searchmix" "searchmix.sqlite" "$serviceTestSrcDir/searchmix"

  parseZippedSource "TinyXmlWithGit" "tinyxmlwithgit" "tinyxmlwithgit.sqlite" "$serviceTestSrcDir/tinyxmlwithgit" "$SERVICE_TEST_DB_DIR/_src/tinyxmlwithgit"
  parseSource "PythonParser" "pythontest" "test_files.sqlite" "$serviceTestSrcDir/pythontest/test_files"
  parseSource "PythonParser" "pythontest2" "st2.sqlite" "$serviceTestSrcDir/pythontest/simple_test2"

  parseJavaSourceDir "SimpleJava" "simplejava" "simplejava.sqlite" "$serviceTestSrcDir/simplejava"
  parseJavaSourceDir "javatest" "javatest" "javatest.sqlite" "$serviceTestSrcDir/javatest"
  parseJavaSourceDir "javademo" "javademo" "javademo.sqlite" "$serviceTestSrcDir/javademo"

  : > "$dirstamp"
}

logD "Service Test environment setup"
logD "  parser command = $parserCommand"
logD "  source root directory = $serviceTestSrcDir"
logD

if ( ! [ -f "$dirstamp" ] ) ||
   [ "$testDirstampPath" -nt "$dirstamp" ] ||
   [ "$parserCommand" -nt "$dirstamp" ] ; then

echo "Generating service test databases..."
  cleanTestDir
  createDatabases
else
  logD "Databases already generated"
fi

logD "Service Test environment setup done"

