#!/bin/bash
#Build environment builder script for CodeCompass.
#author:mcserep@caesar.elte.hu
#author:daniel.krupp@ericsson.com
#author:galex@elte.hu

# SETTINGS

master_server_url=http://plc.inf.elte.hu/model/distfiles
env_file=env.sh


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


# DOWNLOAD & COMPILE FUNCTIONS

#/**
# * @function download
# * @params $1 Archive download url
# * @params $2 Archive file name (default: from URL)
# * @return 0 if freshly downloaded; 1 if already downloaded; 2 if download is prohibited by the user
# * Extra arguments are passed to wget
# */
download()
{
  local archive_download_url=$1
  if [ $# -gt 1 ];
  then
    local archive_file_name=$2
    
    shift 2 # Drop the first two arguments to get access the rest via $@
    local archive_download_options=$@
  else
    local archive_file_name=$(basename "$1")
  fi
  local archive_download_master=$master_server_url/$archive_file_name
  
  if [ "$comm_download" = false ];
  then
    return 2
  fi
  
  echo_info "Downloading $archive_file_name"
  cd $CCMP_DEPS/deps
  if [ ! -f $archive_file_name ];
  then
    echo_info "Fetching $archive_download_master"
    wget --no-check-certificate $archive_download_options -O $archive_file_name $archive_download_master
    if [ $? -ne 0 ];
    then
      echo_warning "Download failed $archive_download_master"
      echo_info "Fetching $archive_file_name $archive_download_url"
      wget --no-check-certificate $archive_download_options -O $archive_file_name $archive_download_url
      if [ $? -ne 0 ];
      then
        echo_error "Download failed $archive_download_url"
        rm -f $archive_file_name
        exit -1
      fi
    fi
    local extension=`echo $archive_file_name | awk -F . '{print $NF}'`
    
    echo_info "Extracting $archive_file_name"
    case "$extension" in
      bz2)
        tar xjf $archive_file_name || exit -1
        ;;
      gz)
        tar xzf $archive_file_name || exit -1
        ;;
      tgz)
        tar xzf $archive_file_name || exit -1
        ;;
      xz)
        tar xJf $archive_file_name || exit -1
        ;;
      zip)
        unzip -u $archive_file_name || exit -1
        ;;
    esac
    return 0
  else
    echo_success "Already downloaded"
    return 1
  fi
}

# Param 1: package name
# Return 0 if should compile, 1 if not
should_build() { 
  local seeking=$1
  local in=1
  for element in "${!comm_packages}"; do
    if [[ $element == $seeking ]]; then
      in=0
      break
    fi
  done
  return $in
}

# Param 1: dependency name
dep() {
  local dep_name=$1
  local avail_check=`declare -F build_$dep_name`
  
  if [ ${#avail_check} -gt 0 ];
  then
    if [ "$comm_force" = true ] || [ "$comm_soft_force" = false ] || should_build $dep_name;
    then
      build_$dep_name
      return $?
    else
      echo_warning "Skip building $dep_name as dependency!"
      return 0;
    fi
  else
    echo_error "Build function $dep_name is not available."
    exit -2
  fi
}

# Creates a wrapper script for a command
#
# param 1: command name (eg.: svn)
# param 2: output file
createCommandWrapper() {
  local realCommand=$(which $1)
  
  if [ -a "$2" ]; then
    #echo "Skipping $2 (already exists)!";
    return 0;
  elif [ -z "$realCommand" ]; then
    echo "WARNING: Command $1 not found!";
    return 0;
  fi

  echo -e \
"#!/bin/bash\n" \
"LD_LIBRARY_PATH='' $realCommand \"\$@\"" > $2
  chmod +x "$2"
}

#/**
# * @function compile
# * @params $1 Compile function name
# * @params $2 Source directory name
# * @params $3 Target binary path relative to CCMP_DEPS
# * @return 0 if freshly compiled; 1 if already compiled; 2 if compilation is prohibited by the user
# * Extra arguments are passed to configure
# */
compile()
{
  local script_name=$1
  local src_dir=$2
  local target_path=$3
  shift 3 # Drop the first three arguments to get access the rest via $@
  local compile_options=$@
  
  if [ "$comm_build" = false ];
  then
    return 2
  fi

  echo_info "Compiling $script_name $src_dir $target_path $config_options"
  cd $CCMP_DEPS/deps/$src_dir
  
  if [ ! -f $CCMP_DEPS/$target_path ] || [ "$comm_force" = true ] || [ "$comm_soft_force" = true ];
  then
    avail_check=`declare -F compile_$script_name`
    if [ ${#avail_check} -gt 0 ];
    then
      compile_$script_name $src_dir $compile_options
      return $?
    else
      echo_error "Compile function $script_name is not available."
      exit -2
    fi
  else
    echo_success "Already compiled"
    return 1
  fi
}

compile_generic()
{
  local src_dir=$1
  shift # Drop the first argument to get access the rest via $@
  local config_options=$@
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  ./configure --prefix=$CCMP_DEPS --libdir=$CCMP_DEPS/lib CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" $config_options || exit -1
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_boost()
{
  local src_dir=$1
  local boost_opts="--without-python --disable-icu boost.locale.icu=off"
  
  echo_info "Cleaning $src_dir"
  ./b2 --clean
  
  echo_info "Configuring $src_dir"
  ./bootstrap.sh --prefix=$CCMP_DEPS --libdir=$CCMP_DEPS/lib || exit -1
  ./b2 -j $cpu_count --prefix=$CCMP_DEPS --libdir=$CCMP_DEPS/lib --build-dir=$CCMP_DEPS $boost_opts || exit -1
  
  echo_info "Making & Installing $src_dir"
  ./b2 install -j $cpu_count --prefix=$CCMP_DEPS --libdir=$CCMP_DEPS/lib $boost_opts || exit -1
  return 0
}

compile_bzip2()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  make -fMakefile-libbz2_so clean

  
  #echo_info "Editing Makefile $src_dir"
  # Required when the compilation of the boost library fails, requesting to recompile the bzip2 library with -fPIC option.
  # sed -i 's/CFLAGS=/CFLAGS=-fPIC /1' Makefile
  
  echo_info "Making & Installing $src_dir"
  make -fMakefile-libbz2_so PREFIX=$CCMP_DEPS || exit -1
  make bzip2 bzip2recover PREFIX=$CCMP_DEPS || exit -1
  
  # MANUAL INSTALLATION
  install -dm755 $CCMP_DEPS/{bin,lib,include,share/man/man1}
  
  install -m755 bzip2-shared $CCMP_DEPS/bin/bzip2
  install -m755 bzip2recover bzdiff bzgrep bzmore $CCMP_DEPS/bin
  ln -sf bzip2 $CCMP_DEPS/bin/bunzip2
  ln -sf bzip2 $CCMP_DEPS/bin/bzcat

  install -m755 libbz2.so.1.0.6 $CCMP_DEPS/lib
  ln -s libbz2.so.1.0.6 $CCMP_DEPS/lib/libbz2.so
  ln -s libbz2.so.1.0.6 $CCMP_DEPS/lib/libbz2.so.1
  ln -s libbz2.so.1.0.6 $CCMP_DEPS/lib/libbz2.so.1.0
  
  install -m644 bzlib.h $CCMP_DEPS/include/

  install -m644 bzip2.1 $CCMP_DEPS/share/man/man1/
  ln -sf bzip2.1 $CCMP_DEPS/share/man/man1/bunzip2.1
  ln -sf bzip2.1 $CCMP_DEPS/share/man/man1/bzcat.1
  ln -sf bzip2.1 $CCMP_DEPS/share/man/man1/bzip2recover.1
  
  return 0
}

compile_cmake()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  ./bootstrap --prefix=$CCMP_DEPS || exit -1
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_gtest()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  find -name \*.py -exec sed -i 's/#!\/usr\/bin\/env python$/#!\/usr\/bin\/env python2/' {} \; 
  ./configure --prefix=$CCMP_DEPS $CONFIG_OPTIONS --libdir=$CCMP_DEPS/lib CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" || exit -1
  # We need to fix the libtool script because there will be syntax error in the generated script; 
  # should rather fix the libtool.m4 file in gtest?
  sed -i "s/\/lib'/lib/g" libtool
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  #echo_info "Installing $src_dir"
  #make install || exit -1
  return 0
}

compile_libgit2()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "CMaking $src_dir"
  mkdir -p build && cd build
  CC="$CCMP_DEPS/bin/gcc"
  CXX="$CCMP_DEPS/bin/g++"
  CMAKE_PREFIX_PATH="$CCMP_DEPS"
  $CCMP_DEPS/bin/cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DBUILD_CLAR=OFF -DTHREADSAFE=ON -DUSE_SSH=OFF -DCMAKE_INSTALL_PREFIX=$CCMP_DEPS -DOPENSSL_SSL_LIBRARY=$CCMP_DEPS
  
  echo_info "Making $src_dir"
  make PREFIX=$CCMP_DEPS -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_thrift()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  find -name \*.py -exec sed -i 's/#!\/usr\/bin\/env python$/#!\/usr\/bin\/env python2/' {} \;
  # TRIFT-2386
  #sed -i 's/^int yylex(void)\;/extern "C" int yylex(void)\;/' compiler/cpp/src/main.h
  ./bootstrap.sh
  ./configure --prefix=$CCMP_DEPS $CONFIG_OPTIONS --libdir=$CCMP_DEPS/lib CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" --with-boost="$BOOST_ROOT" --with-qt4=no --with-csharp=no --with-c_glib=no --with-java=no --with-erlang=no --with-php=no --with-ruby=no --with-haskell=no --with-go=no --with-d=no --with-python=no --with-perl=no --with-lua=no --with-nodejs=no --with-tests=no --enable-libtool-lock PY_PREFIX=$CCMP_DEPS/usr || exit -1
  
  echo_info "Making $src_dir"
  make -j1 || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_xercesc()
{
  local src_dir=$1
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  ./configure --prefix=$CCMP_DEPS $CONFIG_OPTIONS --libdir=$CCMP_DEPS/lib \
    --disable-network --without-icu CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" || exit -1
  # We need to fix the libtool script because there will be syntax error in the generated script; 
  # should rather fix the libtool.m4 file in gtest?
  sed -i "s/\/lib'/lib/g" libtool
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_openssl()
{
  local src_dir=$1
  shift # Drop the first argument to get access the rest via $@
  local config_options=$@
  local openssltarget='';
  local optflags='';

  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  if [ "$arch_type" == 'x86_64' ]; then
		openssltarget='linux-x86_64'
		#optflags='enable-ec_nistp_64_gcc_128'
	elif [ "$arch_type" == 'i686' ]; then
		openssltarget='linux-elf'
	fi

  echo "Target: $openssltarget"
  echo "Opt flags: $optflags"

  ./Configure --prefix=$CCMP_DEPS --libdir=lib --openssldir=$CCMP_DEPS/lib/ssl\
    ${optflags} ${openssltarget} \
    shared no-idea no-mdc2 no-rc5 no-zlib enable-tlsext no-ssl2 \
    "${CPPFLAGS} ${LDFLAGS}" || exit -1

  echo_info "Making $src_dir"
  make depend || exit -1
  make || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1

  return 0
}

compile_perl()
{
  local src_dir=$1
  shift # Drop the first argument to get access the rest via $@
  local config_options=$@
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  ./Configure -des -Dprefix=$CCMP_DEPS -Dnoextensions=ODBM_File || exit -1
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_zlib()
{
  local src_dir=$1
  shift # Drop the first argument to get access the rest via $@
  local config_options=$@
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  ./configure --prefix=$CCMP_DEPS --libdir=$CCMP_DEPS/lib $config_options || exit -1
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

compile_doxygen()
{
  local src_dir=$1
  shift # Drop the first argument to get access the rest via $@
  local config_options=$@
  
  echo_info "Cleaning $src_dir"
  make clean
  
  echo_info "Configuring $src_dir"
  LDFLAGS="$LDFLAGS" ./configure --prefix=$CCMP_DEPS $config_options || exit -1
  
  echo_info "Making $src_dir"
  make -j$cpu_count || exit -1
  
  echo_info "Installing $src_dir"
  make install || exit -1
  return 0
}

# PACKAGE BUILD FUNCTIONS

build_autoconf()
{
  dep m4
  
  download http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
  compile generic autoconf-2.69 bin/autoconf
}

build_automake()
{
  dep autoconf
  
  download http://ftp.gnu.org/gnu/automake/automake-1.15.tar.gz
  compile generic automake-1.15 bin/automake
}

build_boost()
{
  dep bzip2
  dep zlib
  
  download http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.bz2/download boost_1_58_0.tar.bz2
  compile boost boost_1_58_0 lib/libboost_iostreams.so
}

build_bzip2()
{
  dep gcc
  
  download http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
  compile bzip2 bzip2-1.0.6 bin/bzip2
}

build_cmake()
{
  dep gcc
  
  download http://www.cmake.org/files/v3.2/cmake-3.2.3.tar.gz
  compile cmake cmake-3.2.3 bin/cmake
}

build_ctags()
{
  dep gcc
  
  download http://netcologne.dl.sourceforge.net/project/ctags/ctags/5.8/ctags-5.8.tar.gz
  compile generic ctags-5.8 bin/ctags
}

build_file()
{
  dep gcc
  
  download ftp://ftp.astron.com/pub/file/file-5.22.tar.gz
  compile generic file-5.22 bin/file
}

build_gcc()
{
  download http://robotlab.itk.ppke.hu/gcc/releases/gcc-4.9.3/gcc-4.9.3.tar.bz2
  if [ $? -eq 0 ];
  then
    cd $CCMP_DEPS/deps/gcc-4.9.3
    echo_info "Downloading the gcc prerequisites"
    contrib/download_prerequisites || exit -1
  fi
  compile generic gcc-4.9.3 bin/gcc --enable-languages=c,c++ --disable-multilib
}

build_gdb()
{
  dep gcc
  dep ncurses
  
  download http://ftp.gnu.org/gnu/gdb/gdb-7.9.tar.gz
  compile generic gdb-7.9 bin/gdb
}

build_gmp()
{
  dep gcc
  dep m4
  
  download ftp://ftp.gmplib.org/pub/gmp/gmp-6.0.0a.tar.bz2
  compile generic gmp-6.0.0 lib/libgmp.so
}

build_graphviz()
{
  dep gcc
  dep zlib
  
  download http://www.graphviz.org/pub/graphviz/stable/SOURCES/graphviz-2.38.0.tar.gz
  compile generic graphviz-2.38.0 bin/dot --with-webp=no --with-poppler=no \
    --with-rsvg=no --with-ghostscript=no --with-visio=no --with-pangocairo=no \
    --with-lasi=no --with-glitz=no --with-freetype2=no --with-fontconfig=no \
    --with-gdk-pixbuf=no --with-gtk=no --with-gtkgl=no  --with-gtkglext=no \
    --with-gts=no --with-ann=no --with-glade=no --with-ming=no --with-qt=no \
    --with-quartz=no --with-gdiplus=no --with-libgd=no --with-glut=no \
    --without-x --enable-swig=no --enable-sharp=no --enable-guile=no \
    --enable-java=no --enable-lua=no --enable-ocaml=no --enable-perl=no \
    --enable-php=no --enable-python=no --enable-r=no --enable-ruby=no --enable-tcl=no \
    --with-expat=no
}

build_gtest()
{
  dep gcc
  dep python2
  
  download http://github.com/google/googletest/archive/release-1.7.0.zip gtest-1.7.0.zip 
  compile gtest gtest-1.7.0 deps/gtest-1.7.0/lib/libgtest.la
  
}

build_jdk()
{
  download http://download.oracle.com/otn-pub/java/jdk/7u71-b14/jdk-7u71-$arch_for_jdk.tar.gz jdk-7u71-$arch_for_jdk.tar.gz
  if [ $? -eq 0 ];
  then
    cd $CCMP_DEPS/deps/jdk1.7.0_71
    echo_info "Copy JDK ..."
    mkdir -p $CCMP_DEPS/bin $CCMP_DEPS/lib
    cp -rf bin/* jre/bin/* $CCMP_DEPS/bin
    cp -rf lib/* jre/lib/* $CCMP_DEPS/lib
    echo_success "JDK install done."
  fi
}

build_libcutl()
{
  dep boost
  
  download http://www.codesynthesis.com/download/libcutl/1.9/libcutl-1.9.0.tar.gz
  compile generic libcutl-1.9.0 lib/libcutl.so
}

build_libgit2()
{
  dep gcc
  dep cmake
  dep openssl
  dep pkgconfig
  
  download https://github.com/libgit2/libgit2/archive/v0.22.2.tar.gz libgit2-v0.22.2.tar.gz --no-check-certificate
  compile libgit2 libgit2-0.22.2 lib/libgit2.so
}

build_libodb()
{
  dep gcc
  
  download http://www.codesynthesis.com/download/odb/2.4/libodb-2.4.0.zip
  compile generic libodb-2.4.0 lib/libodb.so
}

build_libodb_pgsql()
{
  dep libodb
  dep pgsql
  
  download http://www.codesynthesis.com/download/odb/2.4/libodb-pgsql-2.4.0.zip
  compile generic libodb-pgsql-2.4.0 lib/libodb-pgsql.so
}

build_libodb_sqlite()
{
  dep libodb
  dep sqlite
  
  download http://www.codesynthesis.com/download/odb/2.4/libodb-sqlite-2.4.0.zip
  compile generic libodb-sqlite-2.4.0 lib/libodb-sqlite.so
}

build_libtool()
{
  dep gcc
  
  download http://ftpmirror.gnu.org/libtool/libtool-2.4.6.tar.gz
  compile generic libtool-2.4.6 bin/libtool
}

build_llvm()
{
  dep gcc
  dep zlib
  dep python2
  
  export REQUIRES_RTTI=1
  download http://llvm.org/releases/3.6.0/llvm-3.6.0.src.tar.xz
  download http://llvm.org/releases/3.6.0/cfe-3.6.0.src.tar.xz
  if [ $? -eq 0 ];
  then
    mv ./cfe-3.6.0.src ./llvm-3.6.0.src/tools/clang
    pushd ./llvm-3.6.0.src
    patch -p0 < "${CCMP_SRC}/scripts/patch/comment_lexer_fix.patch"
    patch -p0 < "${CCMP_SRC}/scripts/patch/comment_parser_fix.patch"
    popd
  fi
  download http://llvm.org/releases/3.6.0/compiler-rt-3.6.0.src.tar.xz
  if [ $? -eq 0 ];
  then
    mv ./compiler-rt-3.6.0.src ./llvm-3.6.0.src/projects/compiler-rt
    # sys/vt.h is incompatible with C++ on mtas2
    sed -i -e 's/#include <sys\/vt.h>/\n#define new _new\n#include <sys\/vt.h>\n#undef new/g' \
      ./llvm-3.6.0.src/projects/compiler-rt/lib/sanitizer_common/sanitizer_platform_limits_posix.cc
  fi
  compile generic llvm-3.6.0.src lib/libLLVMCore.a --with-gcc-toolchain=$CCMP_DEPS CC=gcc CXX=g++ --with-python=python2 --disable-terminfo --enable-optimized
}

build_m4()
{
  dep gcc
  
  download http://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.bz2 
  compile generic m4-1.4.17 bin/m4
}

build_ncurses()
{
  dep gcc
  
  # TODO: use offical source
  # Use a snapshot for gcc 5.1, http://trac.sagemath.org/ticket/18301
  download ftp://invisible-island.net/ncurses/ncurses-5.9.tgz
  #download http://ftp.gnu.org/pub/gnu/ncurses/ncurses-5.9.tar.gz
  compile generic ncurses-5.9 lib/libncurses.a --without-ada --with-shared
}

build_odb()
{
  dep libcutl
  dep gmp
  
  download http://www.codesynthesis.com/download/odb/2.4/odb-2.4.0.zip
  # For some reason some headers are searched for in the c-family directory by odb.
  # Create symlinks to them.
  #ln -fs $CCMP_DEPS/lib64/gcc/x86_64-unknown-linux-gnu/4.6.3/plugin/include/*.h $CCMP_DEPS/lib64/gcc/x86_64-unknown-linux-gnu/4.6.3/plugin/include/c-family
 
  # WORKAROUND FOR GCC 4.9.1:
  #if [ $? -eq 0 ];
  #then
  #  download http://codesynthesis.com/~boris/tmp/odb/odb-2.3.0-gcc-4.9.0.patch
  #  pushd ./odb-2.3.0
  #  patch -p1 < "../odb-2.3.0-gcc-4.9.0.patch"
  #  popd
  #fi
 
  # -fno-devirtualize is an anouther workaround for gcc 4.9.x
  CXXFLAGS="$CXXFLAGS -fno-devirtualize" compile generic odb-2.4.0 bin/odb
  #compile generic odb-2.4.0 bin/odb
}

build_openldap()
{
  dep gcc
  dep zlib
  dep openssl
  dep sasl

  download ftp://ftp.openldap.org/pub/OpenLDAP/openldap-release/openldap-2.4.40.tgz
  compile generic openldap-2.4.40 lib/libldap.so --disable-slapd --disable-slurpd
}

build_sasl()
{
  local cpu_count_save=$cpu_count

  dep gcc
  
  download ftp://ftp.cyrusimap.org/cyrus-sasl/cyrus-sasl-2.1.26.tar.gz
  cpu_count=1
  compile generic cyrus-sasl-2.1.26 lib/libsasl2.so --with-dblib=none \
    --disable-cram --disable-digest --disable-otp --disable-krb4 \
    --disable-gssapi --disable-plain --disable-anon
  cpu_count=$cpu_count_save
}

build_readline()
{
  dep gcc
  download ftp://ftp.gnu.org/gnu/readline/readline-6.3.tar.gz
  compile generic readline-6.3 lib/libreadline.so
}

build_pgsql()
{
  dep gcc
  dep readline
  
  download http://ftp.postgresql.org/pub/source/v9.4.1/postgresql-9.4.1.tar.bz2
  compile generic postgresql-9.4.1 bin/psql
}

build_sqlite()
{
  dep gcc
  dep readline

  download https://sqlite.org/2015/sqlite-autoconf-3080803.tar.gz
  CPPFLAGS="$CPPFLAGS -DSQLITE_ENABLE_UNLOCK_NOTIFY" compile generic sqlite-autoconf-3080803 lib/libsqlite3.so
}

build_flex()
{
  dep gcc
  
  download http://optimate.dl.sourceforge.net/project/flex/flex-2.5.39.tar.gz
  compile generic flex-2.5.39 lib/libfl.so
}

build_bison()
{
  dep gcc
  
  download http://ftp.gnu.org/gnu/bison/bison-3.0.4.tar.gz
  compile generic bison-3.0.4 bin/bison
}

build_perl()
{
  dep gcc
  
  # PLEASE DO NOT UPGRADE
  # Openssl does not compiles with a newer version!!!one!!111!!!
  download http://www.cpan.org/src/5.0/perl-5.16.3.tar.gz
  compile perl perl-5.16.3 bin/perl
}

build_openssl()
{
  dep gcc
  dep perl
  dep zlib
  
  download ftp://ftp.openssl.org/source/old/1.0.2/openssl-1.0.2a.tar.gz
  compile openssl openssl-1.0.2a lib/libssl.so
}

build_pkgconfig()
{
  dep gcc

  download http://pkgconfig.freedesktop.org/releases/pkg-config-0.28.tar.gz
  compile generic pkg-config-0.28 bin/pkg-config --with-internal-glib
}

build_libevent()
{
  dep gcc

  download https://sourceforge.net/projects/levent/files/libevent/libevent-2.0/libevent-2.0.22-stable.tar.gz
  compile generic libevent-2.0.22-stable lib/libevent.so
}

build_thrift()
{
  dep boost
  dep flex
  dep bison
  dep openssl
  dep zlib
  dep pkgconfig
  dep libevent
  dep python2
  
  download https://dist.apache.org/repos/dist/release/thrift/0.9.2/thrift-0.9.2.tar.gz
  compile thrift thrift-0.9.2 bin/thrift
}

build_xercesc()
{
  dep gcc
  
  download http://xenia.sote.hu/ftp/mirrors/www.apache.org//xerces/c/3/sources/xerces-c-3.1.1.tar.gz 
  compile xercesc xerces-c-3.1.1 bin/SAX2Print
}

build_zlib()
{
  dep gcc
  
  download http://zlib.net/zlib-1.2.8.tar.gz
  compile zlib zlib-1.2.8 lib/libz.so
}

build_doxygen()
{
  dep gcc
  dep flex
  dep bison
  dep graphviz

  download http://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.9.1.src.tar.gz
  compile doxygen doxygen-1.8.9.1 bin/doxygen "--english-only --static"
}

build_tcsh()
{
  dep gcc
  dep ncurses

  download ftp://ftp.astron.com/pub/tcsh/tcsh-6.18.01.tar.gz
  CFLAGS="-O1" compile generic tcsh-6.18.01 bin/tcsh --disable-nls --disable-nls-catalogs
}

build_python2()
{
  dep gcc
  dep openssl
  dep readline
  dep ncurses
  
  download https://www.python.org/ftp/python/2.7.9/Python-2.7.9.tgz
  compile generic Python-2.7.9 bin/python
}

# ENVIRONMENT GENERATION

#/**
# * @function generate_env
# * @params $1 Environment script file name
# */
generate_env() 
{
  local envf=$1
  echo_info "Preparing environment script to $CCMP_ROOT/$envf"
  
  mkdir -p $CCMP_ROOT || exit -1
  cd $CCMP_ROOT
  
  echo -e "#!/bin/bash\n">$envf
  echo -e \
    "if [ -z \"\$grocker_env_set\" ]; then\n"\
    "  real_svn=\`which svn\`\n"\
    "  alias svn=\"LD_LIBRARY_PATH=\\\"\\\" \$real_svn\"\n\n"\
    "  export CCMP_SRC=\"$CCMP_SRC\"\n"\
    "  export CCMP_DEPS=\"$CCMP_DEPS\"\n"\
    "  export CCMP_ROOT=\"$CCMP_ROOT\"\n"\
    "  export BOOST_ROOT=\"$BOOST_ROOT\"\n"\
    "  export PATH=\"$CCMP_DEPS/bin:$CCMP_ROOT/bin:\$PATH\"\n"\
    "  export LD_LIBRARY_PATH=\"$CCMP_DEPS/lib:$CCMP_ROOT/lib:$CCMP_DEPS/lib64:$CCMP_ROOT/lib64:\$LD_LIBRARY_PATH\"\n"\
    "  export CPPFLAGS=\"$CPPFLAGS\"\n"\
    "  export CXXFLAGS=\"$CXXFLAGS\"\n"\
    "  export LDFLAGS=\"$LDFLAGS\"\n"\
    "  export GTEST_CONFIG=\"$CCMP_DEPS/deps/gtest-1.7.0/scripts/gtest-config\"\n"\
    "  export XERCES_PREFIX=\"$CCMP_DEPS\"\n"\
    "  export SASL_PATH=\"$CCMP_DEPS/lib/sasl2\"\n"\
    "  export MAGIC=\"$CCMP_DEPS/share/misc/magic.mgc\"\n"\
    "  export grocker_env_set=1\n" \
    "  export PS1=(cc)\$PS1\n" \
    "  export GCC_COLORS=1\n" \
    "else\n"\
    "  echo \"Environment already set!\"\n"\
    "fi" >>$envf

  if ((${#MULTIARCH_PATH} > 0));
  then
    echo "
export LIBRARY_PATH=\"\${LIBRARY_PATH}\$(((\${#LIBRARY_PATH} > 0)) && echo \":\")/usr/lib/$MULTIARCH_PATH\"
">>$envf
  fi

  echo_success "Environment script generated"
  echo_info "Source it by executing 'source $envf'"
}


# BUILD SCRIPT

if [ ${#grocker_env_set} -gt 0 ];
then
  echo_error "Do not source $env_file to build tools!"
  exit -2
fi

CCMP_SRC=`pwd`/`dirname $0`
CCMP_SRC=${CCMP_SRC%/.}        # remove the optional './' directory from the end of the path
export CCMP_SRC=${CCMP_SRC%/*} # remove the script directory from the end of the path

if [ -z "$CCMP_DEPS" ]; then
  export CCMP_DEPS=$CCMP_SRC-deps
fi

if [ -z "$CCMP_ROOT" ]; then
  export CCMP_ROOT=$CCMP_SRC-install
fi

export BOOST_ROOT=$CCMP_DEPS
export MULTIARCH_PATH=$(gcc -print-multiarch 2>/dev/null)
export PATH=$CCMP_DEPS/bin:$PATH

export LD_LIBRARY_PATH="$CCMP_DEPS/lib64:$CCMP_DEPS/lib:$LD_LIBRARY_PATH"
export CPPFLAGS="-I$CCMP_DEPS/usr/include -I$CCMP_DEPS/include"
export CFLAGS="-O2"
export CXXFLAGS="-O2"
export LDFLAGS="-L$CCMP_DEPS/usr/lib64 -L$CCMP_DEPS/lib64 -L$CCMP_DEPS/usr/lib -L$CCMP_DEPS/lib"

if ((${#MULTIARCH_PATH} > 0)); then
  export LIBRARY_PATH="${LIBRARY_PATH}$(((${#LIBRARY_PATH} > 0)) && echo ":")/usr/lib/$MULTIARCH_PATH"
  export CPPFLAGS="-I/usr/include/$MULTIARCH_PATH $CPPFLAGS"
fi

echo_info "Exported environment variables:"
echo "CCMP_SRC:        $CCMP_SRC"
echo "CCMP_DEPS:       $CCMP_DEPS"
echo "CCMP_ROOT:       $CCMP_ROOT"
echo "BOOST_ROOT:      $BOOST_ROOT"
echo "MULTIARCH_PATH:  $MULTIARCH_PATH"
echo "PATH:            $PATH"
echo "LIBRARY_PATH:    $LIBRARY_PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "CPPFLAGS:        $CPPFLAGS"
echo "CFLAGS:          $CFLAGS"
echo "CXXFLAGS:        $CXXFLAGS"
echo "LDFLAGS:         $LDFLAGS"
echo

cpu_count=`grep processor /proc/cpuinfo|wc -l`
operating_system=$(uname)
long_bits=$(getconf LONG_BIT)

if [ "$operating_system" == "Linux" ];
then
  if [ $long_bits -eq 64 ]; then
    #export ABI=64
    arch_for_jdk=linux-x64
  else
  if [ $long_bits -eq 32 ]; then
    #export ABI=32
    arch_for_jdk=linux-i586
  else
    echo_error "Internal error: LONG_BITS must be 64 or 32"
    exit 1
  fi
  fi
else
  echo_error "Only the Linux operating system is supported, yet."
  exit 1
fi

arch_type=$(uname -m)

echo_info "Local variables:"
echo "operating_system:  $operating_system"
echo "long_bits:         $long_bits"
echo "arch_for_jdk:      $arch_for_jdk"
echo "arch_type:         $arch_type"
echo "cpu_count:         $cpu_count (compiling thread count)"
echo

echo_info "Settings:"
echo "Master server:       $master_server_url"
echo "Environment script:  $env_file"
echo

mkdir -p $CCMP_DEPS/deps || exit 2
mkdir -p $CCMP_DEPS/bin || exit 2

# Install some wrapper scripts for avoid "no version information available"
# errors
createCommandWrapper svn $CCMP_DEPS/bin/svn
createCommandWrapper mail $CCMP_DEPS/bin/mail
createCommandWrapper awk $CCMP_DEPS/bin/awk
createCommandWrapper gawk $CCMP_DEPS/bin/gawk

comm_download=false
comm_build=false
comm_force=false
comm_soft_force=false
comm_packages=()
avail_packages=()

package_list=`declare -F`
package_list=${package_list//declare -f/}
for package in $package_list
do
  if [[ $package =~ ^build_ ]];
  then
    avail_packages[${#avail_packages[*]}]=${package/build_/}
  fi
done

while [ $# -gt 0 ];
do
  case "$1" in
    -h|--help)
      echo "build_deps - Build environment builder script for CodeCompass."
      echo "build_deps [options], default: build_deps -d -b" 
      echo "options:"
      echo "-h, --help      show brief help"
      echo "-l,             list available packages and exit"
      echo "-d,             download the packages"
      echo "-b,             build the already downloaded packages"
      echo "-f,             force rebuild for the already built packages"
      echo "                (including its dependencies)"
      echo "-r,             force rebuild for the already built packages"
      echo "                (excluding its dependencies)"
      echo "-p [packages],  apply action only for the selected packages"
      exit 0
      ;;
    -l)
      shift
      echo_info "Available packages:"
      for package in ${avail_packages[@]}
      do
        echo $package
      done
      exit 0
      ;;
    -d)
      shift
      comm_download=true
      ;;
    -b)
      shift
      comm_build=true
      ;;
    -f)
      shift
      comm_force=true
      comm_build=true
      ;;
    -r)
      shift
      comm_soft_force=true
      comm_build=true
      ;;
    -p)
      shift
      while [ $# -gt 0 ] && [[ ! "$1" =~ ^- ]];
      do
        comm_packages[${#comm_packages[*]}]=$1
        shift
      done
      ;;
    *)
      echo "Unknown parameter: $1"
      exit 1
      ;;
    esac
done

if [ "$comm_download" = false ] && [ "$comm_build" = false ];
then
  comm_download=true
  comm_build=true
fi

if [ ${#comm_packages[*]} -eq 0 ];
then
  comm_packages=(${avail_packages[@]})
fi

echo_info "Available packages:"
echo ${avail_packages[*]}
echo

echo_info "Install packages:"
echo ${comm_packages[*]}
echo

echo_info "Commands:"
echo "Download:      $comm_download"
echo "Build:         $comm_build"
echo -n "Forced build:  "
if [ "$comm_force" = true ]; then
  echo "yes (with dependencies)"
elif [ "$comm_soft_force" = true ]; then
  echo "yes (without dependencies)"
else
  echo "no"
fi
echo

echo -n "Continue with the above configuration? [yes] "
read comm_start

if [ ${#comm_start} -gt 0 ] && [ ! "$comm_start" = "y" ] && [ ! $comm_start = "yes" ];
then
  echo_warning "Manual stop."
  exit 3
fi

#############################
# Build packages            #
#############################

for package in ${comm_packages[@]}
do
  avail_check=`declare -F build_$package`
  if [ ${#avail_check} -gt 0 ];
  then
    build_$package
  else
    echo_warning "Package $package is not available."
  fi
done

generate_env $env_file

