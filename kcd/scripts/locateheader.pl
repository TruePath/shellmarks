# locateheader.pl - Locate C/C++ header file
# Copyright (c) 2002 Kriang Lerdsuwanakij
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

$prog_name = "locateheader.pl";
$prog_ver = "1.0.2";

$tmp_file_in = "tmp_lh.c";
$tmp_file_out = "tmp_lh.ii";

if (@ARGV < 2) {
	print "$prog_name $prog_ver - Locate C/C++ header file\n";
	print "usage: perl $prog_name COMMAND HEADER\n";
	exit 1;
}

$header = $ARGV[@ARGV-1];
pop @ARGV;
$cmd = join ' ', @ARGV;

open OUTFILE, ">$tmp_file_in" or die "$prog_name: error: cannon open $tmp_file_in for writing\n";
print OUTFILE "#include <$header>\n";
close OUTFILE;

system("$cmd $tmp_file_in > $tmp_file_out");

$header_path = "<unknown>";
open INFILE, $tmp_file_out or die "$prog_name: error: cannot open $tmp_file_out for reading\n";
while (<INFILE>) {
	if (/^#[^\"]*\"(.*)\/$header\"/) {
		if ($header_path eq "<unknown>") {
			$header_path = "$1/$header";
			last;
		}
	}
}

close INFILE;

unlink $tmp_file_in;
unlink $tmp_file_out;

print $header_path;
