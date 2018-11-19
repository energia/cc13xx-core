#!/bin/bash

#  pack.*.bash - Bash script to help packaging samd core releases.
#  Copyright (c) 2015 Arduino LLC.  All right reserved.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

source ./extras/versions.sh 
VERSION=$ENERGIA_VER 
echo $VERSION

PWD=`pwd`
FOLDERNAME=`basename $PWD`
echo $FOLDERNAME
THIS_SCRIPT_NAME=`basename $0`

rm -f $PLATFORM-$VERSION.tar.bz2

cd ..
tar --transform "s|$FOLDERNAME|$FOLDERNAME-$VERSION|g"  --exclude=extras/** --exclude=.git* --exclude=.idea -cjf $PLATFORM-$VERSION.tar.bz2 $FOLDERNAME
cd -

[ -d "extras/build" ] || mkdir extras/build 
mv ../$PLATFORM-$VERSION.tar.bz2 ./extras/build/ 

shasum -a 256 extras/build/$PLATFORM-$VERSION.tar.bz2 > extras/build/$PLATFORM-$VERSION.tar.bz2.sha256
#stat -f%z $PLATFORM-$VERSION.tar.bz2


#read -r -p "Are you sure? [Y/n]" response
#response=$(echo "$response" | tr '[:upper:]' '[:lower:]')
#if [[ $response =~ ^(yes|y| ) ]]; then
#    scp $PLATFORM-$VERSION.tar.bz2 robertinant@energia.nu:html/cores/
#fi
