#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;
use File::Compare;
use File::Basename;

my $iterations=0;
my $noprint = 0;
my $help = 0;

GetOptions (
            "help"          => \$help,
            "noprint"       => \$noprint,
            "iterations:s"    => \$iterations,
            ) or die("Error in command line arguments\n");

run_tests();
aggreagate();

sub run_tests {
  for(my $i = 0 ; $i < $iterations; $i++) {
    print "Iteration: " . $i . "\n";
    my $outpath = "./profiles/$i" . ".prof";
    my $backup = "./profiles/$i" . ".back";

    execute("lit -v -j 8 . -o $outpath");
    execute("grep \"exec_time\\|\\\"name\\\":\" $outpath \> $backup");
    execute("mv $backup $outpath");
  }
}

sub aggreagate {
  my $fp;
  my $inputFile = "profiles/testcases.txt";

  open ($fp, "<", $inputFile) or die "cannot open < $inputFile: $!";
  my @lines = <$fp>;
  my $numFunctions = scalar(@lines);
  close ($fp);

  my $i = 0;
  for my $line  (@lines) {
    chomp $line;
    my $backup = "./profiles/$i" . ".back";
    my $outfile = "./profiles/$i" . ".csv";
    execute("grep -B 1 \"$line\" profiles/\*\.prof | grep -v \"\\-\\-\" | grep -v \"\\\"name\\\":\" | sed -e \"s/\.\*: //g\" | tr \'\\n\' \' \'  > $backup");
    $i++;
    execute("mv $backup $outfile");
  }

  for (my $i = 0 ; $i < $numFunctions; $i++) {
    my $inputFile = "profiles/$i.csv";
    open ($fp, "<", $inputFile) or die "cannot open < $inputFile: $!";
    my @lines = <$fp>;
    close ($fp);
    my $line = $lines[0];
    $line = $i. ", " . "$line";
    execute("echo $line >> profiles/profile.csv");
    execute("rm -rf $inputFile");
  }

}

# Utilities
sub execute {
  my $args = shift @_;
  if(0 == $noprint) {
    print "$args \n";
  }
  system("$args");
}

sub help {

}
