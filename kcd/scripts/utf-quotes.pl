# utf-quotes.pl - Generate UTF-8 nice quotes
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


print "# Automatically generated by utf-quotes.pl\n";

$translate = 0;
while (<STDIN>) {
	$line = $_;
	$done = "";
	if ($line =~ /^\s*msgid/) {
		$translate = 0;
	}
	elsif ($line =~ /^\s*msgstr/) {
		$translate = 1;
	}

						# Skip comment lines
						# and lines containing eval `...`
	if ($translate == 1 && $line !~ /^\#/ && $line !~ /eval \`.*\`/) {
		while ($line ne "") {
						# Find first occurance of either
						# ` or '
			if ($line =~ /([\`\'])/) {
				$pre = $`;
				$left = $1 . $';
				if ($1 eq "`") {
						# ` found first
					if ($left =~ /^\`\`\'/) {
						$done = $done . $pre .
							"\xe2\x80\x98`\xe2\x80\x99";
						$line = $';
					}
					elsif ($left =~ /^\`\'\'/) {
						$done = $done . $pre .
							"\xe2\x80\x98\'\xe2\x80\x99";
						$line = $';
					}
					elsif ($left =~ /^\`/) {
						$done = $done . $pre . "\xe2\x80\x98";
						$line = $';
					}
				}
				else {
						# ' found first
					if ($left =~ /^\'/) {
						$done = $done . $pre . "\xe2\x80\x99";
						$line = $';
					}
				}
			}
			else {
				$done = $done . $line;
				$line = "";
			}
		}
	}
	else {
		$done = $_;
	}
	print $done;
}
