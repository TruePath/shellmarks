# copy-message.pl - Copy message from msgid to msgstr
# Copyright (c) 2000, 2002 Kriang Lerdsuwanakij
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

#
# Program information
#

$prog_name = "copy-message.pl";
$prog_ver = "";
$description = "Copy message from msgid to msgstr";
$copyright = "Copyright (c) 2000, 2002 Kriang Lerdsuwanakij";

#
# Main program
#

sub parse_arguments;
sub subst_string;

%var_hash = ();
parse_arguments(@ARGV);

print "# Automatically generated by copy-message.pl\n";

@msgid_list = ();
$begin_multiline_msgid = 0;
$in_multiline_msgid = 0;

while (<STDIN>) {

			#
			# Process line immediately after the string
			# 'msgid ""'.  This requires special treatment
			# only if it is immediately followed by
			# 'msgstr ""'.
			#

	if ($begin_multiline_msgid) {
		if (/^\s*msgstr/) {		# Information
			print $_;
			$begin_multiline_msgid = 0;
			$in_multiline_msgid = 0;
		}
		elsif (/^\s*\"/) {		# Continue multiline msgid
			print $_;
			push @msgid_list, $_;
			$begin_multiline_msgid = 0;
		}
		else {
			print $_;
			$begin_multiline_msgid = 0;
			$in_multiline_msgid = 0;
		}
	}

			#
			# Process all other lines after the string
			# msgid "".  If 'msgstr' is found, we expand
			# with string collected earlier.
			#

	elsif ($in_multiline_msgid) {
		if (/^\s*\"/) {			# Continue multiline msgid
			print $_;
			push @msgid_list, $_;
		}
		elsif (/^\s*msgstr/) {		# Start msgstr
			$output_tag = 0;
			for $line (@msgid_list) {
				if ($output_tag == 0) {
					print "msgstr" . "$line";
					$output_tag = 1;
				}
				else {
					print $line;
				}
			}
			$in_multiline_msgid = 0;
		}
		else {
			print $_;
			$in_multiline_msgid = 0;
		}
	}

			#
			# Beginning of 'msgid' cases
			#

	elsif (/^\s*msgid\s*\"\"/) {
		print $_;
		@msgid_list = ();
		push @msgid_list, " \"\"\n";
		$begin_multiline_msgid = 1;
		$in_multiline_msgid = 1;
	}
	elsif (/^\s*msgid/) {
		print $_;
		@msgid_list = ();
		push @msgid_list, $';
		$begin_multiline_msgid = 0;
		$in_multiline_msgid = 1;
	}

			#
			# Other cases, comments, etc.
			#

	else {
		if (/# SOME DESCRIPTIVE TITLE/
		    && exists($var_hash{TITLE})) {
			print "# $var_hash{TITLE}\n";
		}
		elsif (/\# Copyright \(C\) YEAR Free Software Foundation\, Inc/
		       && exists($var_hash{COPYRIGHT})) {
			print "# $var_hash{COPYRIGHT}\n";
		}
		elsif (/\# FIRST AUTHOR \<EMAIL\@ADDRESS\>\, YEAR/
		       && exists($var_hash{AUTHOR})) {
			print "# $var_hash{AUTHOR}\n";
		}
		elsif (/\#\, fuzzy/) {
			;
		}
		elsif (/Project-Id-Version/) {
			s/PACKAGE/$var_hash{PACKAGE}/ if exists($var_hash{PACKAGE});
			s/VERSION/$var_hash{VERSION}/ if exists($var_hash{VERSION});
			print $_;
		}
		elsif (/PO-Revision-Date/) {
			s/YEAR\-MO\-DA HO\:MI\+ZONE/$var_hash{DATE}/ if exists($var_hash{DATE});
			print $_;
		}
		elsif (/Last-Translator/) {
			s/FULL NAME \<EMAIL\@ADDRESS\>/$var_hash{TRANSLATOR}/ if exists($var_hash{TRANSLATOR});
			print $_;
		}
		elsif (/Language-Team/) {
			s/LANGUAGE \<LL\@li.org\>/$var_hash{LANGUAGE_TEAM}/ if exists($var_hash{LANGUAGE_TEAM});
			print $_;
		}
		elsif (/Content-Type/) {
			s/CHARSET/$var_hash{CHARSET}/ if exists($var_hash{CHARSET});
			print $_;
		}
		else {
			print $_;
		}
	}
}

#
# Subroutines
#

sub display_version {
	print <<EOF;
$prog_name $prog_ver
EOF
	exit 0;
}

sub display_help {
	print <<EOF;
$prog_name $prog_ver - $description
$copyright

Usage:   perl $prog_name [OPTION]...

Available options:
     -d VAR VALUE   Define variable VAR initialized with VALUE.
     -f FILE        Load variables from FILE.
     -h, --help     Display this help.
     -v, --version  Display version number.
EOF
	exit 0;
}

sub set_variable {
	my @parm = @_;
	die "$prog_name: invalid load_file call\n" if scalar(@parm) != 2;
	$var_hash{uc($parm[0])} = $parm[1];
}

sub load_file {
	my @parm = @_;
	die "$prog_name: invalid load_file call\n" if scalar(@parm) != 1;

	open (INFILE, $parm[0]) or die "$prog_name: cannot open $parm[0]\n";
	while (<INFILE>) {
		if (/^\s*(\S*)\s*=\s*\"(.*)\"/) {
			set_variable ($1, $2);
		}
	}
}

sub parse_arguments {
	my @parm = @_;
	my $i;
	my $num_i = scalar (@parm);

	for ($i = 0; $i < $num_i; $i++) {
		if ($parm[$i] eq "-h" || $parm[$i] eq "--help") {
			display_help();
		}
		elsif ($parm[$i] eq "-v" || $parm[$i] eq "--version") {
			display_version();
		}
		elsif ($parm[$i] eq "-f" && $i < $num_i-1) {
			load_file($parm[$i+1]);
			$i++;
		}
		elsif ($parm[$i] eq "-d" && $i < $num_i-2) {
			set_variable($parm[$i+1], $parm[$i+2]);
			$i += 2;
		}
		else {
			die "$prog_name: unrecognized option $parm[$i]\n";
		}
	}
}
