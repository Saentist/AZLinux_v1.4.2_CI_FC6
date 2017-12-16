#!/usr/bin/perl
use strict;
use Date::Parse;

#################################################################
# analyse diffs

my $in = shift;
my $line;
my $subject;
my $sub_ok=0;
my $init=0;
my $num=0;
my $hgimport=0;
my $mmimport=0;
my $maint_ok=0;
my $noblank=1;
my $maintainer_name=$ENV{CHANGE_LOG_NAME};
my $maintainer_email=$ENV{CHANGE_LOG_EMAIL_ADDRESS};
my $from="";
my $body="";
my $signed="";
my $fromname="";

open IN,  "<$in";

while ($line = <IN>) {
	if ($line =~ m/Index.*/) {
		last;
	}
	if ($line =~ m/^diff .*/) {
		last;
	}
	if ($line =~ m/^\-\-\- .*/) {
		last;
	}
	if ($line =~ m/^\-\-\-\-.*/) {
		$body="";
		next;
	}
	if ($line =~ m/^\-\-\-.*/) {
		last;
	}
	if ($line =~ m/^\+\+\+ .*/) {
		last;
	}

	if ($line =~ m/^Date:\s*(.*)/) {
		my $time = str2time($1);
#		my $timestr = gmtime($time);

		if ($time) {
			print "# Date: $time\n";
		}
		next;
	}
	if ($line =~ m/^From:/) {
		if ($line =~ m/^From:[\s\"]*([^\"]*)[\s\"]*<(.*)>/) {
			if ($1 eq "") {
				next;
			}
			my $name=$1;
			my $email=$2;
			$name =~ s/\s+$//;
			$email =~ s/\s+$//;
			$fromname="$name <$email>";
			$from= "From: $fromname\n";
			next;
		}
		if ($line =~ m/^From:\sakpm\@osdl.org/) {
			$mmimport=1;
			next;
		}
		print "Bad: author line have a wrong syntax: $line\n";
		die;
	}

	if ($line =~ m/^Subject:\s*(.*\n)/) {
		$subject=$1;
		next;
	}

	if ($line =~ m;^ .*\/.*\| *[0-9]*;) {
		next;
	}
	if ($line =~m/\d+\s*file.* changed, /) {
		next;
	}

	if ($line =~ m/^Signed-off-by:.*/) {
		$noblank=1;
		if ($line =~ m/$maintainer_name/) {
			$maint_ok=1;
		}

		$signed="$signed$line";
		next;
	}
	if ( ($line =~ m/^\# HG changeset patch/) ||
	     ($line =~ m/^has been added to the -mm tree.  Its filename is/) ) {
		$sub_ok=0;
		$init=0;
		$num=0;
		$maint_ok=0;
		$noblank=1;
		$from="";
		$body="";
		$subject="";
		$hgimport=1;
		next;
	}


	if ($line =~ m/^Acked-by:.*/) {
		$signed="$signed$line";
		next;
	}

	if ($line =~ m/^[a-zA-Z\-]*:/) {
		if ($line =~ m/Changeset:\s*(.*)\n/) {
			$num=$1;
		}
#		print "# $line";
		next;
	}

	if ($line =~ m|^(V4L\/DVB\s*\(.+\)\s*:.*\n)|) {
		$subject=$1;
		$line="\n";
	}

	if ($line =~ m/^#/) {
		next;
	}
	if ($sub_ok == 0) {
		$sub_ok=1;
		substr( $subject, 0, 1 ) = uc (substr ($subject, 0, 1));
		if ($subject =~ m|V4L\/DVB\s*(.+)|) {
			$subject=$1;
		}
		if ($hgimport) {
			$subject=$line;
			next;
		}
		if ($line =~ m/^\n/) {
			next;
		}
	}

	if ($noblank) {
		if ($line =~ m/^\n/) {
			next;
		}
	}
	if (!$init) {
		$init=1;
		$noblank=0;
	}
	$body="$body$line";
}
close IN;

if ($from eq "") {
	print "Bad: author doesn't exist!\n";
	die;
}

if (!$maint_ok) {
	$signed=$signed."Signed-off-by: $maintainer_name <$maintainer_email>\n";
}

if (!$signed =~ m/$from/) {
	print "Bad: Author didn't signed his patch!\n";
	die;
}

$body=~s/[\n\s]+$//;
$body=~s/^[\n\s]+//;

# First from is used by hg to recognize commiter name
print "#Committer: $maintainer_name <$maintainer_email>\n";
print "$subject\n$from\n$body\n\n$signed";

