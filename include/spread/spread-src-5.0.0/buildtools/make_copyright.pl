#!/usr/bin/perl -w

# Make the Copyright notices in Spread appear and disappear

sub sub_notice_from_file()
{
    my ($filename, $notice_file) = @_;
    my ($newfilename, $line, $ln, @notice_lines, $mode, $first_line);
    
    open(NOTICE, "< $notice_file") or die "can't open $notice_file: $!";
    @notice_lines = <NOTICE>;
    close(NOTICE);

    if (@notice_lines < 2 || $notice_lines[@notice_lines - 1] !~ /^[ \t]*\*\/[ \t]*$/) {
      die "$notice_file length was too small or didn't end with a closing comment!";
    }

    open(IN, "< $filename") or die "can't open $filename: $!";
    $mode = (stat IN)[2];
    @file_lines = <IN>;
    close(IN);

    $newfilename = $filename . ".new";
    open(OUT, "> $newfilename") or die "can't open $filename.new: $!";

    if (-f $newfilename && ! -w $newfilename) {
        chmod(0600, $newfilename) or die "Failed to make $newfilename Writable: $!\n";
    }

    # print file until we match against first two lines of NOTICE

    for ($ln = 0; $ln < @file_lines && ($ln + 1 == @file_lines || $file_lines[$ln] ne $notice_lines[0] || $file_lines[$ln + 1] ne $notice_lines[1]); $ln++) 
    {
        print OUT $file_lines[$ln];
    }

    # skip lines until we find an end comment that matches last line of NOTICE

    for (; $ln < @file_lines && $file_lines[$ln] !~ /^[ \t]*\*\/[ \t]*$/; $ln++) 
    {}

    # print rest of file

    for (++$ln; $ln < @file_lines; $ln++) 
    {
        print OUT $file_lines[$ln];
    }

    close(OUT);
    chmod ($mode, $newfilename) or die "chmod failed: $!";
    rename( $filename, "$filename.bak1") or die "Can't rename $filename to $filename.bak1\n";
    rename( $newfilename, $filename) or die "can't rename $newfilename to $filename\n";

    return 1;
}
    
sub add_notice_to_file()
{
    my ($filename, $notice_file) = @_;
    my ($newfilename, $line, @notice_lines, $mode);
    
    open(NOTICE, "< $notice_file") or die "can't open $notice_file: $!";
    @notice_lines = <NOTICE>;
    close(NOTICE);

    $newfilename = $filename . ".new";
    open(IN, "< $filename") or die "can't open $filename: $!";
    open(OUT, "> $newfilename") or die "can't open $filename.new: $!";

    $mode = (stat IN)[2];
    if (-f $newfilename && ! -w $newfilename) {
	chmod(0600, $newfilename) or print "Failed to make $newfilename Writable: $!\n";
    }
    foreach $line (@notice_lines) {
	print OUT $line;
    }
    while(<IN>)
    {
	print OUT $_;
    }

    close(IN);
    close(OUT);
    chmod ($mode, $newfilename) or die "chmod failed: $!";
    rename( $filename, "$filename.bak2") or die "Can't rename $filename to $filename.bak2\n";
    rename( $newfilename, $filename) or die "can't rename $newfilename to $filename\n";
    return 1;
}

use Getopt::Long;

$tgt_opt = "./";

GetOptions( "sub=s" => \$sub_opt,
	    "add=s" => \$add_opt,
            "tgt=s" => \$tgt_opt );

#main function

if ( -d $tgt_opt )
{
    opendir(DIR, $tgt_opt) or die "can't opendir $tgt_opt: $!";

    while (defined($file = readdir(DIR))) 
    {
        if ( ($file !~ /^\./ ) && ( 
                 ( $file =~ /\.[ch]$/) ||
                 ( $file =~ /\.java$/) ) ) 
        {
            $file = $tgt_opt . "/" . $file;
            print "About to act on file: $file\n";

            if ( $sub_opt ) 
            {
                print "Removing notice from... $file\n";
                &sub_notice_from_file($file, $sub_opt);
            }

            if ( $add_opt ) 
            {
                print "Adding notice to... $file\n";
                &add_notice_to_file($file, $add_opt);
            }
        } 
        else 
        {
            print "Skipping... $file\n";
        }
    }

    closedir(DIR);
}
elsif ( -f $tgt_opt )
{
    $file = $tgt_opt;
    print "About to act on file: $file\n";
    
    if ( $sub_opt ) 
    {
        print "Removing notice from... $file\n";
        &sub_notice_from_file($file, $sub_opt);
    }
    
    if ( $add_opt ) 
    {
        print "Adding notice to... $file\n";
        &add_notice_to_file($file, $add_opt);
    }
}
else
{
    print "$tgt_opt is neither a file nor a directory... skipping!\n";
}
