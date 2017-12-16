#!/usr/bin/perl
use FileHandle;

my $kernel=shift;
my $instdir = ();

sub check_line($$$)
{
	my $dir=shift;
	my $obj=shift;
	my $arg=shift;
	my $arg2="";

	my $idir = $dir;

	$idir =~ s|^../linux/drivers/media/||;

	$arg=$arg." ";

	while ($arg ne "") {
		if ($arg =~ m|^([^ /]+)/ |) {
			my $newdir=$1;
#			print "include $dir/$newdir/Makefile\n";
			open_makefile("$dir/$newdir/Makefile");
			$arg =~ s/^[^ ]+ //;
		} else {
			$arg =~ s/^([^ ]+ )//;
			my $cur=$1;
			$arg2 = $arg2.$cur;

			$cur =~ s/\s+$//;
			$cur =~ s/\\$//;
			$cur =~ s/\s+$//;
			$cur =~ s/\.o$/.ko/;

			if ( ($cur ne "") && (! ($instdir { $idir } =~ m/($cur) /))) {
				$instdir { $idir } = $instdir { $idir }.$cur." ";
			}
		}
	}
	$arg2 =~ s/\s+$//;
	if ($arg2 ne "") {
#			print "arg=$arg2\n";
		print OUT "$obj$arg2\n";
	}
}

sub remove_includes($$)
{
	my $obj=shift;
	my $arg=shift;
	my $arg2="";

	$arg=$arg." ";

	while ($arg ne "") {
		if (!$arg =~ m|^(-I\s*[^ ]+) |) {
			$arg2 = $arg2.$1;
			$arg =~ s|^[^ ] ||;
		} else {
			$arg =~ s|^(-I\s*[^ ]+) ||;
		}
	}
	if ($arg2 ne "") {
		print OUT "$obj$arg2\n";
	}
}

sub open_makefile($) {
	my $file= shift;
	my $in = new FileHandle;
	my $dir= "";
	my $count=0;

	$file=~ m|^(.*)/[^/]*$|;
	$dir=$1;

#print "opening $file (dir=$dir)\n";
	open $in,"$file";

	while (<$in>) {
		if (m|\s*#|) {
			print OUT $_;
			next;
		}
		if (m|^\s*include|) {
			next;
		}
		if (count==1 || m|(^\s*EXTRA_CFLAGS.*=\s*)(.*/.*)\n|) {
			remove_includes($1,$2);
			if (m|[\\]\n|) {
				$count=1;
			} else {
				$count=0;
			}
			next;
		}
#		if (m|(^\s*obj.*=\s*)(.*/.*)\n|) {
		if (count==2 || m|(^\s*obj.*=\s*)(.*)\n|) {
			check_line($dir,$1,$2);
			if (m|\\\n|) {
				$count=1;
			} else {
				$count=0;
			}
			next;
		}
		print OUT $_;
	}
	close $in;
}

open OUT,">Makefile.media.new";
open_makefile ("../linux/drivers/media/Makefile");

# Creating Install rule
printf OUT "media-install::\n";
printf OUT "\t\@echo \"Stripping debug info from files:\"\n";
printf OUT "\t\@strip --strip-debug \$(inst-m)\n\n";

while ( my ($key, $value) = each(%instdir) ) {
	printf OUT "\t\@echo -e \"\\nInstalling \$(KDIR26)/$key files:\"\n";
	printf OUT "\t\@install -d \$(KDIR26)/$key\n";

	printf OUT "\t\@files='$value'; ";
	printf OUT "for i in \$\$files;do if [ -e \$\$i ]; then echo -n \"\$\$i \";";
	printf OUT " install -m 644 -c \$\$i \$(KDIR26)/$key; fi; done; echo;\n\n";
}
printf OUT "\t/sbin/depmod -a \${KERNELRELEASE}\n\n";

# Creating Remove rule
printf OUT "media-rminstall::\n";
printf OUT "\t\@echo -e \"\\nRemoving old \$(DEST) files\\n\"\n";

while ( my ($key, $value) = each(%instdir) ) {
	printf OUT "\t\@echo -e \"\\nRemoving old \$(KDIR26)/$key files:\"\n";
	printf OUT "\t\@files='$value'; ";

	printf OUT "for i in \$\$files;do if [ -e \$(KDIR26)/$key/\$\$i ]; then ";
	printf OUT "echo -n \"\$\$i \";";
	printf OUT " rm \$(KDIR26)/$key/\$\$i; fi; done; ";

	printf OUT "for i in \$\$files;do if [ -e \$(KDIR26)/$key/\$\$i.gz ]; then ";
	printf OUT "echo -n \"\$\$i.gz \";";
	printf OUT " rm \$(KDIR26)/$key/\$\$i.gz; fi; done; echo;\n\n";
}

close OUT;

while ( my ($key, $value) = each(%config) ) {
	delete $depend{$key};
}

open OUT,">Kconfig.kern";
while ( my ($key, $value) = each(%depend) ) {
	print OUT "# $key with $value refs\nconfig $key\n\ttristate\n\tdefault m\n\n";
}
close OUT;

if (open IN,"Makefile.media") {
	close IN;
	my $changed=0;
	if (open IN,"diff Makefile.media Makefile.media.new|") {
		while (<IN>) {
			if ($_ ne "") {
				$changed=1;
			}
		}
		close IN;
		if ($changed) {
			printf("One or more linux Makefiles had changed. Makefile.media rewrited.\n");
			system ("mv Makefile.media.new Makefile.media");
		} else {
			system ("rm Makefile.media.new");
		}
	}
} else {
	printf("Creating Makefile.media.\n");
	system "mv Makefile.media.new Makefile.media";
}

if (open IN,".myconfig") {
	close IN;
} else {
	system "./scripts/make_kconfig.pl $kernel 1";
}
