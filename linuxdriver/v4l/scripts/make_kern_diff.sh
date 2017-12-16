KERNEL=/usr/src/linux-2.6.12-rc5-mm

KDIR="`find $KERNEL/drivers/media/ $KERNEL/include/media -type d|sed s,$KERNEL/,,g`"

echo $KDIR

CUR="`pwd`"
mkdir kernel
cd kernel
for d in $KDIR; do
	mkdir -p $d
	FILES="`ls $KERNEL/$d|grep -v Makefile`"

	for f in $FILES; do
		if [ -e $CUR/$f ]; then
			echo ln -sf $CUR/$f $d/$f
			ln -sf $CUR/$f $d/$f
		fi
	done
done

mkdir -p Documentation/video4linux
FILES="`find $CUR/doc/* -type f`"
for f in $FILES; do
	echo ln -sf $f Documentation/video4linux/
	ln -sf $f Documentation/video4linux/

done

cd $CUR
# Excludes DVB code from diff
rm -r kernel/drivers/media/dvb

(export LC_ALL=C;diff -ur /usr/src/linux-2.6.12-rc5-mm kernel/|grep -v Only)
