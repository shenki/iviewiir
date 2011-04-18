#!/bin/bash

DEVKITPRO_DIR=/opt/devkitpro
DEVKITPPC_DIR="$DEVKITPRO_DIR"/devkitPPC
export PATH="$PATH":"$DEVKITPPC_DIR"/bin

ZLIB_NAME=zlib-1.2.5.tar.bz2
POLARSSL_NAME=polarssl-0.14.0-gpl.tgz
LIBRTMP_NAME=rtmpdump-2.3.tgz
JSON_C_NAME=json-c-0.9.tar.gz
LIBXML2_NAME=libxml2-sources-2.7.8.tar.gz

TLD="`pwd`" # top level directory

# setup zlib
[ -e "$ZLIB_NAME" ] || \
    wget http://zlib.net/"$ZLIB_NAME" || \
    exit 1
ZLIB_DIR="${ZLIB_NAME%.tar.bz2}"
[ ! -d "$ZLIB_DIR" ] || rm -rf "$ZLIB_DIR" || exit 1
tar -xvjf "$ZLIB_NAME"
pushd "$ZLIB_DIR"
CC=powerpc-eabi-gcc \
    AR=powerpc-eabi-ar \
    RANLIB=powerpc-eabi-ranlib \
    LD=powerpc-eabi-ld \
    prefix="$DEVKITPRO_DIR"/portlibs/ppc \
    ./configure --static
sed -i -e '/cp \$(SHAREDLIBV) \$(DESTDIR)\$(sharedlibdir)/d' Makefile
make && make install || exit 1
popd

# setup PolarSSL
 install PolarSSL pkgconfig file
POLARSSL_PKGCONFIG=/opt/devkitpro/portlibs/ppc/lib/pkgconfig
[ -e $polarssl_pkgconfig ] || \
    wget http://jms.id.au/~shenki/wii/polarssl.pc -O $POLARSSL_PKGCONFIG || \
    exit 1
POLARSSL_DIR=${POLARSSL_NAME%-gpl.tgz}
[ -e "$POLARSSL_NAME" ] || \
    wget "http://polarssl.org/code/releases/$POLARSSL_NAME" || \
    exit 1
[ -e polarssl-0.14.0.patch ] || \
    wget http://jms.id.au/~shenki/wii/polarssl-0.14.0.patch || \
    exit 1
[ ! -d "$POLARSSL_DIR" ] || rm -rf "$POLARSSL_DIR" || exit 1
tar xvzf "$POLARSSL_NAME" || exit 1
pushd "$POLARSSL_DIR"
rm library/net.c
patch -p1 < "$TLD"/polarssl-0.14.0.patch || exit 1
sed -i -e "s:^\(DESTDIR=\).*$:\1/$DEVKITPRO_DIR/portlibs/ppc:" Makefile
pushd library
make || exit 1
popd
make install || exit 1
popd

# setup librtmp
LIBRTMP_DIR=${LIBRTMP_NAME%.tgz}
[ -e "$LIBRTMP_NAME" ] || \
    wget http://rtmpdump.mplayerhq.hu/download/"$LIBRTMP_NAME" || \
    exit 1
[ -e librtmp-2.3.patch ] || \
    wget http://jms.id.au/~shenki/wii/librtmp-2.3.patch || \
    exit 1
[ ! -d "$LIBRTMP_DIR" ] || rm -rf "$LIBRTMP_DIR" || exit 1
tar xvf "$LIBRTMP_NAME" || exit 1
pushd "${LIBRTMP_DIR}"
patch -p1 < "$TLD"/librtmp-2.3.patch || exit 1
pushd librtmp
make install CROSS_COMPILE=powerpc-eabi- INC=-I/opt/devkitpro/portlibs/ppc/include || exit 1
popd
popd

# setup json-c
JSON_C_DIR="${JSON_C_NAME%.tar.gz}"
[ -e $JSON_C_NAME ] || \
    wget http://ftp.debian.org/debian/pool/main/j/json-c/json-c_0.9.orig.tar.gz -O $JSON_C_NAME || \
    exit 1
[ ! -d "$JSON_C_DIR" ] || rm -rf "$JSON_C_DIR" || exit 1
tar xvf "$JSON_C_NAME"
pushd "$JSON_C_DIR"
patch -p1 < "$TLD"/json-c-0.9.patch
CC=powerpc-eabi-gcc CFLAGS="-DGEKKO -DRVL -mrvl -mcpu=750 -meabi -mhard-float" \
    ./configure --prefix=/opt/devkitpro/portlibs/ppc --host=powerpc-eabi \
    --disable-shared --enable-static
sed -i -e '/rpl_\(m\|re\)alloc/d' config.h
sed -i -e 's/\(HAVE_VSYSLOG \)1/\10/' config.h
make && make install || exit 1
popd

# setup libxml
LIBXML2_DIR="${LIBXML2_NAME%.tar.gz}"
LIBXML2_DIR="${LIBXML2_DIR/-sources/}"
[ -e "$LIBXML2_NAME" ] || \
    wget ftp://xmlsoft.org/libxml2/"$LIBXML2_NAME" || \
    exit 1
[ ! -d "$LIBXML2_DIR" ] || rm -rf "$LIBXML2_DIR" || exit 1
tar xvf "$LIBXML2_NAME" || exit 1
pushd "$LIBXML2_DIR"
patch -p1 < "$TLD"/libxml2-2.7.8.patch
CFLAGS="-DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -I/opt/devkitpro/portlibs/ppc/include" \
      LDFLAGS="-L/opt/devkitpro/portlibs/ppc/lib -L/opt/devkitpro/libogc/lib/wii" \
      CC=powerpc-eabi-gcc ./configure --host=powerpc-eabi --enable-static --disable-shared \
      --without-c14n --without-catalog --without-debug --without-docbook \
      --without-fexceptions --without-ftp --without-history --without-html \
      --without-iso8859x --without-legacy --without-mem-debug --without-minimum \
      --without-output --without-pattern --without-push --without-reader \
      --without-regexps --without-run-debug --without-http --without-schemas \
      --without-schematron --without-threads --without-thread-alloc --without-tree \
      --without-valid --without-writer --without-xinclude --without-xpath \
      --without-xptr --without-modules --without-coverage --without-python \
      --prefix=/opt/devkitpro/portlibs/ppc --without-iconv || exit 1
make libxml2.la
make install-data
make install-libLTLIBRARIES
popd
echo
echo Done!
echo
