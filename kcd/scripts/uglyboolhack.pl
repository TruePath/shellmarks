# uglyboolhack.pl - Make ncurses header C++ compatible
# Copyright (c) 2000 Kriang Lerdsuwanakij
# email:	lerdsuwa@users.sourceforge.net
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

$prog_name = "uglyboolhack.pl";
$prog_ver = "1.0.0";

if (@ARGV != 2) {
	print "$prog_name $prog_ver - Make ncurses header C++ compatible\n";
	print "usage: perl uglyboolhack.pl INFILE OUTFILE\n";
	exit 1;
}

open INFILE, $ARGV[0] or die "$prog_name: error: cannot open $ARGV[0] for reading\n";
open OUTFILE, ">$ARGV[1]" or die "$prog_name: error: cannot open $ARGV[1] for writing\n";
print "creating $ARGV[1] from $ARGV[0]\n";
print OUTFILE "/*\n    Created from $ARGV[0] by $prog_name $prog_ver\n*/\n";

$new_bool_type = "<unknown>";
while (<INFILE>) {
	if ($new_bool_type eq "<unknown>") {
		if (/typedef (.*) bool/) {
			$new_bool_type = $1;
		}
	}
	else {
		s/bool/$new_bool_type/g;
	}
	print OUTFILE $_;
}

close INFILE;
close OUTFILE;

if ($new_bool_type eq "<unknown>") {
	print "$prog_name: error: bool replacement failed, cannot determine how ncurses defines bool\n";
	print "$prog_name: error: compiled program will not work properly!\n";
}
else {
	print "bool successfully replaced by $new_bool_type in $ARGV[1]\n";
}
