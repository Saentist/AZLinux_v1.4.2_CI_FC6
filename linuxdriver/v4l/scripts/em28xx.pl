#!/usr/bin/perl -w
use strict;

my $new_entry = -1;
my $nr = 0;
my ($id,$subvendor,$subdevice);
my %data;

while (<>) {
	# defines in header file
	if (/#define\s+(EM2820_BOARD_\w+)\s+(\d+)/) {
		$data{$1}->{nr} = $2;
		next;
	}
	if (/#define\s+(EM2800_BOARD_\w+)\s+(\d+)/) {
		$data{$1}->{nr} = $2;
		next;
	}
	# em2820_boards
	if (/\[(EM2820_BOARD_\w+)\]/) {
		$id = $1;
		$data{$id}->{id} = $id;
		$data{$id}->{type} = "(em2820/em2840)";
#		$data{$id}->{nr} = $nr++;
	};
	if (/\[(EM2800_BOARD_\w+)\]/) {
		$id = $1;
		$data{$id}->{id} = $id;
		$data{$id}->{type} = "(em2800)";
#		$data{$id}->{nr} = $nr++;
	};

	next unless defined($id);

	if (/USB_DEVICE.*0x([0-9a-fA-F]*).*0x([0-9a-fA-F]*).*driver_info.*(EM2820_BOARD_\w+)/ ) {
		$subvendor=$1;
		$subdevice=$2;
                push @{$data{$3}->{subid}}, "$subvendor:$subdevice";
	}

	if (/USB_DEVICE.*0x([0-9a-fA-F]*).*0x([0-9a-fA-F]*).*driver_info.*(EM2800_BOARD_\w+)/ ) {
		$subvendor=$1;
		$subdevice=$2;
                push @{$data{$3}->{subid}}, "$subvendor:$subdevice";
	}


        if (!defined($data{$id}) || !defined($data{$id}->{name})) {
                $data{$id}->{name} = $1 if (/\.name\s*=\s*\"([^\"]+)\"/);
        }

	# em2820_USB_tbl


}

foreach my $item (sort { $data{$a}->{nr} <=> $data{$b}->{nr} } keys %data) {
	printf("%3d -> %-40s %-15s", $data{$item}->{nr}, $data{$item}->{name},$data{$item}->{type});
	printf(" [%s]",join(",",@{$data{$item}->{subid}}))
	  if defined($data{$item}->{subid});
	print "\n";
}
