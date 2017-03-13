#
#  Copyright 2017 by Texas Instruments Incorporated.
#  
#

#!/bin/sh
#
#
tmp=".tmp$$"
rm -rf $tmp; mkdir $tmp

XDCROOT=$TOOLS/vendors/xdc/xdctools_3_20_05_76/Linux/
NM=$TOOLS/vendors/ti/c6x/6.1.0/Linux/bin/nm6x

TEST="java -classpath java/package.jar:$XDCROOT/packages/xdc/rta/java/package.jar ti.targets.omf.elf.Elf32"

for f in `ls tests/repo/sanity/hello.x*`; do
    addr=`$NM $f | grep ' msg$' | cut -d" " -f1`
    b=`basename $f`
    $TEST $f 0x"$addr" > $tmp/$b.log
    if diff $tmp/$b.log output/$b.log; then
        echo "    $f passed"
        rm $tmp/$b.log
    else
        echo "    $f failed"
        status="fail"
    fi
done

if [ "$status" = "fail" ]; then
    echo `ls $tmp | wc -l` regressions failed.
    exit 1
fi

echo all regressions passed.
rm -rf $tmp
exit 0
#
#  @(#) ti.targets.omf.elf; 1,0,0,0; 2-10-2017 09:26:14; /db/ztree/library/trees/xdctargets/xdctargets-m11/src/ xlibrary

#

