#!/bin/sh
# link drivers sources from CVS or release tarball into your 2.6.x kernel sources;

if test -z $1 || ! test -d $1 ; then
	echo
	echo "  usage: $0 <path_to_kernel_to_patch>"
	echo
	exit
fi

echo "patching $1..."

cd linux
PWD=`pwd`

for x in `find drivers -type d | grep -v CVS` ; do
	mkdir -p -v $1/$x
done

for x in `find Documentation -type d | grep -v CVS` ; do
	mkdir -p -v $1/$x
done

for x in `find include -type d | grep -v CVS` ; do
	mkdir -p -v $1/$x
done

for x in `find Documentation -type f | grep -v CVS | grep -v .cvsignore` ; do
	ln -f -s $PWD/$x $1/$x
done

for x in `find drivers -type f | grep -v CVS | grep -v .cvsignore` ; do
	ln -f -s $PWD/$x $1/$x
done

for x in `find include -type f | grep -v CVS | grep -v .cvsignore` ; do
	ln -f -s $PWD/$x $1/$x
done

for x in `find include -type d | grep -v CVS` ; do
	ln -f -s $PWD/../v4l/compat.h $1/$x/compat.h
	touch $1/$x/config-compat.h
done

for x in `find drivers/media -type d | grep -v CVS` ; do
	ln -f -s $PWD/../v4l/compat.h $1/$x/compat.h
	touch $1/$x/config-compat.h
done

cd ..
patch -p0 <<'DIFF'
diff -u -p videodev.h
--- linux/include/linux/videodev.h
+++ linux/include/linux/videodev.h
@@ -1,6 +1,7 @@
 #ifndef __LINUX_VIDEODEV_H
 #define __LINUX_VIDEODEV_H

+#include "compat.h"
 #include <linux/types.h>

 #define HAVE_V4L1 1
