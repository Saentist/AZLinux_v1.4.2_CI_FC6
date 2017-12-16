#!/usr/bin/perl
use FileHandle;

my %depend = ();
my %minver = ();
my %config = ();
my %intopt = ();
my %hexopt = ();
my %tristate = ();
my %kernopts = ();
my %depmods = ();
my $version, $level, $sublevel, $kernver;

my $kernel=shift;
my $force_kconfig=shift;

#!/usr/bin/perl
use FileHandle;

###########################################################
# Checks config.h and autoconf.h for current kernel defines
sub process_config ($)
{
	my $filename = shift;
	my $in = new FileHandle;

	open $in,"$kernel/include/$filename" or die "File not found: $kernel/include/$filename";
	while (<$in>) {
		if (m/\#define\s+CONFIG_([^ ]*)_ON_SMP\s+(.*)\n/) {
			my $key=$1;
			my $value=$2;
#			printf "defined $key as $value\n";
			if ( $value == 1 ) {
				$value='y';
			}
			$kernopts{$key}=$value;
			next;
		}
		if (m/\#define\s+CONFIG_([^ ]*)_MODULE\s+(.*)\n/) {
			my $key=$1;
			my $value=$2;
#			printf "defined $key as $value\n";
			if ( $value == 1 ) {
				$value='m';
			}
			$kernopts{$key}=$value;
			next;
		}
		if (m/\#define\s+CONFIG_([^ ]*)\s+(.*)\n/) {
			my $key=$1;
			my $value=$2;
#			printf "defined $key as $value\n";
			if ( $value == 1 ) {
				$value='y';
			}
			$kernopts{$key}=$value;
			next;
		}
		if (m/\#undef\s+CONFIG_([^ ]*)\n/) {
#			printf "undefined $1\n";
			$kernopts{$1}='n';
			next;
		}
	}

	close $in;
}

sub add_bool($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown boolean '$arg'";
	$tristate{$arg}="bool";
#	printf "Boolean:%s\n",$arg;

	$kernopts{$arg}='y';
}

sub add_tristate($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown tristate '$arg'";
	$tristate{$arg}="tristate";

	$kernopts{$arg}='m';
}

sub add_int($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown int '$arg'";
	$intopt{$arg} = 0;
}

sub add_hex($)
{
	my $arg=shift;

	exists $config{$arg} or die "Adding unknown hex '$arg'";
	$hexopt{$arg} = 0;
}

sub set_int_value($$)
{
	my $key = shift;
	my $val = shift;

	exists $intopt{$key} or die "Default for unknown int option '$key'";
	$intopt{$key} = $val;
}

sub set_hex_value($$)
{
	my $key = shift;
	my $val = shift;

	exists $hexopt{$key} or die "Default for unknown hex option '$key'";
	$hexopt{$key} = "0x".$val;
}

sub add_config($)
{
	my $arg=shift;

	if ($arg =~ m/^(\w+)/) {
		# Have option default to 'on'
		$config{$1} = 1;
	} else {
	    die "Do not understand config variable '$arg'";
	}
}

########################################
# Turn option off, iff it already exists
sub disable_config($)
{
	my $key = shift;

	$config{$key} = 0 if (exists $config{$key});
}

#################################################################
# Make a list of dependencies and the number of references for it
sub check_deps($$)
{
	my $key=shift;
	my $arg=shift;

	$depmods{$key}=$arg;
	$arg=$arg." ";

	while ($arg ne "") {
		if ($arg =~ m/^([A-Z0-9_]+) /) {
			my $val=$1;
			my $prev=$depend{$val};
			$depend { $val } = 1+$prev;
		}
		$arg =~ s/^[^ ]+ //;
	}

	return $ok;
}

######################################################
# Checks if all dependencies for the key are satisfied
sub deps_ok($)
{
	my $key=shift;
	my $arg=$depmods{$key};
	my $ok=1;

	if ($arg eq "") {
		return $ok;
	}

	$arg=$arg." ";


#	printf "$key: deps are '$arg'\n";

	while ($arg ne "") {
		if ($arg =~ m/^([A-Z0-9_]+) /) {
			if ((! exists $kernopts {$1}) || ($kernopts {$1} eq 'n')) {
				printf "$key: Required kernel opt '$1' is not present\n";
				$ok=0;
			}
		}
		if ($arg =~ m/^\!([A-Z0-9_]+) /) {
			if ($kernopts {$1} eq 'y') {
				printf "$key: Driver is incompatible with  '$1'\n";
				$ok=0;
			}
		}
		$arg =~ s/^[^ ]+ //;
	}

	return $ok;
}

sub open_kconfig($$) {
	my ($dir,$file)=@_;
	my $in = new FileHandle;
	my $disabled=0;
	my $key;

#print "opening $file\n";
	open $in,"$file" or die "File not found: $file";
	while (<$in>) {
#		if (m;^\s*source[\s\"]+drivers/media/(video|dvb)/Kconfig;) {
#			next;
#		}

		# start of a new stanza, reset
		if (m/^\w/) {
			$disabled = 0;
			$default_seen = 0;
			$key = undef;
		}

		if (m|^\s*source\s+"([^"]+)"\s*$| ||
		    m|^\s*source\s+(\S+)\s*$|) {
			open_kconfig($dir,"$dir/$1");
			next;
		}
		if (m|^\s+depends on\s+(.+?)\s*$|) {
			check_deps ($key,$1);
		}
		if (m|^\s+select\s+(.+?)\s*(if .*?)?\s*$|) {
			check_deps ($key,$1);
		}
		if (m|^\s+bool(ean)?\s|) {
			add_bool($key);
		}
		if (m|^\s+tristate\s|) {
			add_tristate($key);
		}
		if (m|^\s+int\s|) {
			add_int($key);
		}
		if (m|^\s+hex\s|) {
			add_hex($key);
		}
		# Get default for int options
		if (m|^\s+default "(\d+)"| && exists $intopt{$key}) {
			set_int_value($key, $1);
		}
		# Get default for hex options
		if (m|^\s+default "(0x)?([[:xdigit:]]+)"| && exists $hexopt{$key}) {
			set_hex_value($key, $2);
		}
		# Override default for disabled tri/bool options
		if (m/^\s+default (y|n|m|"yes"|"no")\s+(if .*)?$/ &&
		    exists $tristate{$key} && $disabled) {
			$default_seen = 1;
			$_ = "\tdefault n\n";
		}
		# check for end of config definition for disabled drivers
		# we need to make sure we've disabled it, and add a bit
		# to the help text
		if (m|^\s*(---)?help(---)?\s*$| && $disabled) {
			if(exists $tristate{$key} && !$default_seen) {
				print OUT "\tdefault n\n";
			}
			print OUT <<"EOF";
	depends on VIDEO_KERNEL_VERSION
	help
	  WARNING! This driver needs at least kernel $minver{$key}!  It may not
	  compile or work correctly on your kernel, which is too old.

EOF
			next;
		}

		if (m|^\s*config (\w+)\s*$|) {
			$key=$1;
			add_config ($1);

			if (exists $minver{$key} &&
			    cmp_ver($minver{$key}, $kernver) > 0) {
				$disabled=1;
				disable_config ($key);
				print "$key requires version $minver{$key}\n";
				print OUT "# $key disabled for insufficient kernel version\n";
			} else {
#				printf "OK: $key requires version %s\n", exists $minver{$key}?$minver{$key}:"any";
				$disabled=0;
			}
		}

		s/^main(menu\s\"[^\"]+)/\1 - DON'T CHANGE IT!/;

		print OUT $_;
	}
	close $in;
}

sub parse_versions ()
{
	my $in = new FileHandle;
	my $ver;

	open $in,"versions.txt" or die "File not found: versions.txt";
	while (<$in>) {
		if (/\[(\d+\.\d+\.\d+)\]/) {
			$ver = $1;
		} elsif (/^\s*(\w+)/) {
			$minver{$1} = $ver;
#			print "$1=$ver\n";
		}
	}
	close $in;
}

# Like ver1 <=> ver2, but understands X.Y.Z version strings
sub cmp_ver($$)
{
	shift =~ /(\d+)\.(\d+)\.(\d+)/;
	my ($v1_ver,$v1_level,$v1_sublevel) = ($1,$2,$3);
	shift =~ /(\d+)\.(\d+)\.(\d+)/;
	my ($v2_ver,$v2_level,$v2_sublevel) = ($1,$2,$3);

	my $cmp = $v1_ver <=> $v2_ver;
	return $cmp unless($cmp == 0);
	$cmp = $v1_level <=> $v2_level;
	return $cmp unless($cmp == 0);
	return $v1_sublevel <=> $v2_sublevel;
}

process_config("linux/autoconf.h");

parse_versions;

open IN,".version" or die "File not found: .version";
while (<IN>) {
	if (m/KERNELRELEASE\s*[:]*[=]+\s*(\d+)\.(\d+)\.(\d+)/) {
		$version=$1;
		$level=$2;
		$sublevel=$3;
		$kernver="$version.$level.$sublevel";
	}
}
close IN;

print "Preparing to compile for kernel version $kernver\n";

open OUT,">Kconfig" or die "Cannot write Kconfig file";

print OUT <<"EOF";
mainmenu "V4L/DVB menu"
source Kconfig.kern
config VIDEO_KERNEL_VERSION
	bool "Enable drivers not supported by this kernel"
	default n
	---help---
	  Normally drivers that require a kernel newer $version.$level.$sublevel,
	  the kernel you are compiling for now, will be disabled.

	  Turning this switch on will let you enabled them, but be warned
	  they may not work properly or even compile.

	  They may also work fine, and the only reason they are listed as
	  requiring a newer kernel is that no one has tested them with an
	  older one yet.

	   If the driver works, please post a report at V4L mailing list:
			video4linux-list\@redhat.com.

	  Unless you know what you are doing, you should answer N.

EOF

open_kconfig ("../linux","../linux/drivers/media/Kconfig");
close OUT;

while ( my ($key, $value) = each(%config) ) {
		delete $depend{$key};
}

open OUT,">Kconfig.kern" or die "Cannot write Kconfig.kern file";

while ( my ($key, $value) = each(%depend) ) {
	if ($kernopts{$key}) {
		print OUT "# $key with $value refs\nconfig $key\n\ttristate\n\tdefault ".
			$kernopts{$key}."\n\n";
	} else {
		print OUT "# $key with $value refs\nconfig $key\n\ttristate\n\tdefault n #not found\n\n";
	}
}
close OUT;

# These options should default to off
disable_config('DVB_AV7110_FIRMWARE');
disable_config('DVB_CINERGYT2_TUNING');
disable_config('DVB_FE_CUSTOMISE');

# Hack for check sound/oss/aci.h header

my $mirodep="$kernel/sound/oss/aci.h";
if (! -e $mirodep) {
	my $key="RADIO_MIROPCM20";
	print <<"EOF2";
$key: $mirodep is missing.

***WARNING:*** You do not have the full kernel sources installed.
This does not prevent you from building the v4l-dvb tree if you have the
kernel headers, but the full kernel source is required in order to use
make menuconfig / xconfig / qconfig.

If you are experiencing problems building the v4l-dvb tree, please try
building against a vanilla kernel before reporting a bug.

Vanilla kernels are available at http://kernel.org.
On most distros, this will compile a newly downloaded kernel:

cp /boot/config-`uname -r` <your kernel dir>/.config
cd <your kernel dir>
make all modules_install install

Please see your distro's web site for instructions to build a new kernel.

EOF2
	$kernopts{$key}='n';
}

# Recursively check for broken dependencies
my $i;
do {
	$i=0;
	while ( my ($key,$value) = each(%kernopts) ) {
		if ($value ne 'n') {
			if (!deps_ok($key)) {
				$kernopts{$key}='n';
			}
			$i=$i+1;
		}
	}
} until (!$disable);

# Produce a .config file if it's forced or one doesn't already exist
if (($force_kconfig eq 1) || !open IN,".config") {
	open OUT,">.config" or die "Cannot write .config file";
	while ( my ($key,$value) = each(%tristate) ) {
		if (!$config{$key}) {
			print OUT "# CONFIG_$key is not set\n";
		} elsif ($kernopts{$key}) {
			if ($kernopts{$key} eq 'n') {
				print OUT "# CONFIG_$key is not set\n";
			} else {
				print OUT "CONFIG_$key=".$kernopts{$key}."\n";
			}
		} elsif ($value eq 'tristate') {
			print OUT "CONFIG_$key=m\n";
		} else { # must be 'bool'
			print OUT "CONFIG_$key=y\n";
		}
	}
	while ( my ($key,$value) = each(%intopt) ) {
		print OUT "CONFIG_$key=$value\n";
	}
	while ( my ($key,$value) = each(%hexopt) ) {
		print OUT "CONFIG_$key=$value\n";
	}
	close OUT;
}
