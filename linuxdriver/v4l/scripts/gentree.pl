#!/usr/bin/perl
use strict;
use File::Find;
use Fcntl ':mode';

my $VER = shift;
my $SRC = shift;
my $DESTDIR = shift;
my $extra;


#$VER = "2.6.14"			if !defined($VER);
#$SRC   = "../linux"		if !defined($SRC);
#$DESTDIR = "/tmp/linux"		if !defined($DESTDIR);

if (!defined($DESTDIR)) {
	print "Usage:\ngentree.pl\t<version> <source dir> <dest dir>\n\n";
	exit;
}

my ($VERSION,$CODE) = &kernel_version;
my $DEBUG = 0;


#################################################################
# helpers

sub kernel_version() {
	my ($ver,$code);

	$code = 0;
	$ver = $VER;

	$ver =~ m/(\d)\.(\d)\.([0-9]*(.*))/;
	my $v1 = $1;
	my $v2 = $2;
	my $v3 = $3;
	$extra = $4;

	$code = $v1*65536 + $v2*256 + $v3;

	return ($ver,$code);
}

#################################################################
# filter out version-specific code

sub filter_source ($$) {
	my ($in,$out) = @_;
	my ($line,$mmkernel);
	my $level=0;
	my %if = ();
	my %state =();
	my $print=0;

	if ($extra =~ m/\-mm/) {
		$mmkernel = 1;
	} else {
		$mmkernel = 0;
	}

	open IN,  "<$in";
	open OUT, ">$out";

	print STDERR "File: $in, for kernel $VERSION($CODE)/\n" if $DEBUG;

	while ($line = <IN>) {
		if ($line =~ m/^#include \"compat.h\"/) {
			next;
		}
#		if ($line =~ m/[\$]Id:/) {
#			next;
#		}
		if ($line =~ /^#ifdef MM_KERNEL/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = $mmkernel;
			print STDERR "/* BP #if MM_KERNEL state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifdef _COMPAT_H/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 0;
			print STDERR "/* BP #if MM_KERNEL state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ m|^\s*\#if\s+0\s*\/[\*]\s*[Kk][Ee].[Pp]\s*[\*]\/(.*)|) {
			if ($1 eq ";") {
				print OUT "#if 0\n";
			} else {
				print OUT "#if 0$1\n";
			}
			chomp($line);
			$state{$level} = "ifother";
			$if{$level} = 0;
			print STDERR "/* BP #if 0 (keep) state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifdef\sSTV0297_CS2/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 0;
			print STDERR "/* BP #if 0 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifndef\sBROKEN_XAWTV/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 0 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifndef\sSTV0297_CS2/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if 0/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 0;
			print STDERR "/* BP #if 0 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ m|^\s*\#if\s+1\s*\/[\*]\s*[Kk][Ee].[Pp]\s*[\*]\/(.*)|) {
			print OUT "#if 1$1\n";
			$state{$level} = "ifother";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 (keep) state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if 1/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*HAVE_VIDEO_BUF_DVB/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*CONFIG_XC3028/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 0;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*I2C_CLASS_TV_ANALOG/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifndef.\s*I2C_DF_DUMMY/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifndef.\s*I2C_PEC/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 0;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#ifdef.\s*I2C_PEC/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*I2C_CLASS_TV_DIGITAL/) {
			chomp($line);
			$state{$level} = "if";
			$if{$level} = 1;
			print STDERR "/* BP #if 1 state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*BTTV_VERSION_CODE/) {
			chomp($line);
			$line =~ s@^#if\s*@@;
			$line =~ s@BTTV_VERSION_CODE@\$CODE@;
			$line =~ s@KERNEL_VERSION\((\d+),\s*(\d+),\s*(\d+)\)@
					sprintf("%d",$1*65536 + $2*256 + $3) @e;
			$if{$level} = eval $line;
			$state{$level} = "if";
			print STDERR "/* BP #if BTTV_VERSION_CODE state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if.*LINUX_VERSION_CODE/) {
			chomp($line);
			$line =~ s@^#if\s*@@;
			$line =~ s@LINUX_VERSION_CODE@\$CODE@;
print STDERR "/* BP orig: ($line) */\n" if $DEBUG;

			$line =~ s@KERNEL_VERSION\((\d+),\s*(\d+),\s*(\d+)\)@
					sprintf("%d",$1*65536 + $2*256 + $3) @e;
			$if{$level} = eval $line;
			$state{$level} = "if";
			print STDERR "/* BP #if LINUX_VERSION_CODE state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
			next;
		}
		if ($line =~ /^#if\s/) {
			$state{$level} = "ifother";
			$if{$level} = 1;
			print STDERR "/* BP $line state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
		}

		if ($line =~ /^#ifdef/) {
			$state{$level} = "ifother";
			$if{$level} = 1;
			print STDERR "/* BP $line state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
		}

		if ($line =~ /^#ifndef/) {
			$state{$level} = "ifother";
			$if{$level} = 1;
			print STDERR "/* BP $line state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			$level++;
		}

		if ($level) {
			if ($state{$level-1} eq "if" && $line =~ /^#else/) {
				$state{$level-1} = "else";
				print STDERR "/* BP #else state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
				next;
			}
		}
		if ($line =~ /^#endif/) {
			if ($level) {
				$level--;

				print STDERR "/* BP #endif state=$state{$level} if=$if{$level} level=$level ($line) */\n" if $DEBUG;
			} else {
				 die "more endifs than ifs";
			}
			if ($state{$level} ne "ifother") {
				next;
			}
		}
		if ($level == 0) {
			print OUT $line;
		} else {
			my $i=0,$print=1;
			for ($i=0;$i<$level;$i++) {
				if (!($state{$i} eq "if"   &&  $if{$i} ||
				      $state{$i} eq "else" && !$if{$i}) &&
				      $state{$i} ne "ifother" ) {
					$print=0;
					chomp($line);
					print STDERR "/* BP DEL $line state=$state{$i} if=$if{$i} level=$i */\n" if $DEBUG;
				}
			}

			if ($print) {
				print OUT $line;
			}
		}
	}
	close IN;
	close OUT;
}

sub filter_dest ($$) {
	my ($in,$out) = @_;
	my ($line,$if,$state,$mmkernel);

	if ($extra =~ "/\-mm/") {
		$mmkernel = 1;
	} else {
		$mmkernel = 0;
	}

	open IN,  "<$in";
	open OUT, ">$out";

	while ($line = <IN>) {
		if ($line =~ m/[\$]Id:.*/) {
			next;
		}
		print OUT $line;
	}
	close IN;
	close OUT;
}

#################################################################

sub parse_dir {
	my $file = $File::Find::name;
	my $srcdir=$SRC;

	if ($file =~ /CVS/) {
		return;
	}

	if ($file =~ /\~$/) {
		return;
	}

	my $mode = (lstat("$file"))[2];

	if ($mode & S_IFDIR) {
		return;
	}

	$srcdir =~ s/(.)/\[$1\]/g;
	my $f2 = $file;

	$f2 =~ s,^$srcdir,$DESTDIR/,;
	print "from $file to $f2\n";

	my $tmp = "/tmp/src.$$";
	if ($file =~ m/.*\.[ch]$/) {
		filter_source("$file","$tmp");
	} else {
	system("cp $file $tmp");
	}

	my $dir = $f2;
	$dir =~ s,(.*)[/][^/]*$,$1,;

	print("mkdir -p $dir\n");
	system("mkdir -p $dir\n");
	system("cp $tmp $f2");
	unlink $tmp;
}


# main

my $patchtmploc = "/tmp/temp.patch";

printf <<EOF,$VER,$CODE;
kernel is %s (0x%x)
EOF

print "finding files at $SRC\n";


find(\&parse_dir, $SRC);
