#!/bin/bash
#Build script for CodeComapss.
#author:mcserep@caesar.elte.hu
#author:daniel.krupp@ericsson.com

# INFORMATION FUNCTIONS

echo_info()
{
  echo -e "\e[1;34m$1\e[0m"
}

echo_warning()
{
  echo -e "\e[1;33m$1\e[0m"
}

echo_error()
{
  echo -e "\e[1;31m$1\e[0m"
}

echo_success()
{
  echo -e "\e[1;32m$1\e[0m"
}


# BUILD SCRIPT

if [ ${#grocker_env_set} -eq 0 ]; then
  echo_error "Load the CodeCompass build environment first!"
  exit -2
fi

echo_info "Exported environment variables:"
echo "CCMP_SRC:        $CCMP_SRC"
echo "CCMP_DEPS:       $CCMP_DEPS"
echo "CCMP_ROOT:       $CCMP_ROOT"
echo "LDFLAGS:         $LDFLAGS"
echo

cpu_count=`grep processor /proc/cpuinfo|wc -l`

echo_info "Local variables:"
echo "cpu_count:  $cpu_count (compiling thread count)"
echo

comm_build=false
comm_install=false
comm_clean=false
comm_reconfigure=false

database_type="pgsql"
database_indices=""

while [ $# -gt 0 ];
do
  case "$1" in
    -h|--help)
      echo "build_cc - Build CodeCompass."
      echo "build_cc [options], default: build_cc -b -i"
      echo "options:"
      echo "-h, --help  show brief help"
      echo "-b,         build"
      echo "-i,         install"
      echo "-f,         force a clean build"
      echo "-o,         force reconfigure"
      echo "-c,         do a 'make clean' end exit"
      echo "-sqlite,    configure with sqlite database (use with -o)"
      exit 0
      ;;
    -o)
      comm_reconfigure=true
      shift
      ;;
    -c)
      comm_clean=true
      shift
      ;;
    -f)
      comm_clean=true
      comm_build=true
      shift
      ;;
    -b)
      comm_build=true
      shift
      ;;
    -i)
      comm_install=true
      shift
      ;;
    -sqlite)
      database_type="sqlite"
      database_indices="--disable-indexes"
      shift
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
  esac
done

if [ "$comm_build" = false ] && [ "$comm_install" = false ] && [ "$comm_reconfigure" = false ] && [ "$comm_clean" = false ];
then
  comm_build=true
  comm_install=true
fi

echo_info "Commands:"
echo    "Clean:        $comm_clean"
echo    "Build:        $comm_build"
echo    "Install:      $comm_install"
echo -n "Reconfigure:  $comm_reconfigure"
if [ "$comm_reconfigure" = true ];
then
    echo -n " ($database_type)"
fi
echo
echo

echo -n "Continue with the above configuration? [yes] "
read comm_start

if [ ${#comm_start} -gt 0 ] && [ ! "$comm_start" = "y" ] && [ ! $comm_start = "yes" ];
then
  echo_warning "Manual stop."
  exit 3
fi

if [ ! -d $CCMP_ROOT ];
then
  echo_warning "$CCMP_ROOT does not exist"
  mkdir -p $CCMP_ROOT || exit -1
fi
cd $CCMP_ROOT

cd $CCMP_SRC
if [ "$comm_clean" = true ];
then
  make clean
  if [ "$comm_build" = false ];
  then
    exit 0
  fi
fi

if [ ! -f configure ];
then
  echo_info "Autogen CodeCompass"
  ./autogen.sh || exit -1
  comm_reconfigure=true
fi
if [ "$comm_reconfigure" = true ];
then
  echo_info "Configuring CodeCompass"
  ./configure --with-database=$database_type $database_indices CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" --prefix=$CCMP_ROOT
fi

if [ "$comm_build" = true ];
then
  echo_info "Making CodeCompass"
  make -j$cpu_count || exit -1
  make || exit -1
  echo_success "CodeCompass successfully compiled"
fi

if [ "$comm_install" = true ];
then
  echo_info "Installing CodeCompass"
  make install || exit -1
  echo_success "CodeCompass successfully installed"
fi
