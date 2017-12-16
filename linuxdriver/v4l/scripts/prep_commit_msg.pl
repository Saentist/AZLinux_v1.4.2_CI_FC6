#!/usr/bin/perl

my $autopatch = shift;

# Get Hg username from environment
my $user = $ENV{HGUSER};

# Didn't work? Try the .hgrc file
if ($user eq "") {
	open IN, "<$ENV{HOME}/.hgrc";
	while (<IN>) {
		if(/^\s*username\s*=\s*(\S.*)$/) {
			$user = $1;
			last;
		}
	}
	close IN;
}

# Still no luck? Try some other environment variables
if ($user eq "") {
	my $name = $ENV{CHANGE_LOG_NAME};
	my $email = $ENV{CHANGE_LOG_EMAIL_ADDRESS};
	$user = "$name <$email>" if ($name ne "" || $email ne "");
}

# Last try to come up with something
if ($user eq "") {
	$user = "$ENV{USER} <>";
}

print "# Added/removed/changed files:\n";
system "hg diff | diffstat -p1 -c";
if (-s $autopatch) {
	print "#\n# Note, a problem with your patch was detected!  These changes were made\n";
	print "# automatically: $autopatch\n";
	system "diffstat -p0 -c $autopatch";
	print "#\n# Please review these changes and see if they belong in your patch or not.\n";
}
print <<"EOF";
#
# For better log display, please keep a blank line after subject, after from,
# and before signed-off-by.
# First line should be the subject, without Subject:
#


# Now, patch author (just the main one), on a From: field
# Please change below if the committer is not the patch author.
#
From: $user

# Then a detailed description:


# At the end Signed-off-by: fields by patch author and committer, at least.
#
Signed-off-by: $user
EOF
