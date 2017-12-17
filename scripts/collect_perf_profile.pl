#!/usr/bin/env perl

use strict;
use warnings;
use Getopt::Long;
use File::Compare;
use File::Basename;
use File::Find;
use Cwd;

my $topn=0;
my $help = 0;
my $verbose=0;

GetOptions (
            "help"          => \$help,
            "topn:s"    => \$topn,
            "verbose"   => \$verbose,
            ) or die("Error in command line arguments\n");

aggregate_perfdata();


# Utilities
sub help {}

sub aggregate_perfdata {
  my %func_map;
  my $totalTicks = 0;

  my $prof_files_ref = collect_perf_record_files();

  foreach my $file  (@$prof_files_ref) {
    chomp $file;
    my $outfile = $file . ".report";
    print "create report file: $outfile". "\n" if $verbose == 1;
    execute("perf report --stdio -i  $file --sort=overhead,comm,symbol  > $outfile");
    parse_perf_report($outfile, \%func_map);
    #execute("rm -rf $outfile");
  }

  #normalize_profile(aggregatedProfile, totalTicks)
  dump_aggregated_profile(\%func_map);
}

sub parse_perf_report {
    my $inputfile = shift @_;
    my $func_map_ref = shift @_;
    my $sample_count = 0;

    print "Parse report file: ", $inputfile, "\n" if $verbose == 1;
    my $fp;
    open ($fp, "<", $inputfile) or die "cannot open < $inputfile: $!";
    my @lines = <$fp>;
    close ($fp);

    my $count = 0;
    for my $line (@lines) {
      chomp $line;
      $line =~ m/^#.*/;
      if($line =~ m/^# Event count \(approx\.\): (\d+)/) {
        $sample_count =  $1;
        next;
      }
      if($line =~ m/^#.*/) {
        next;
      }

      $line =~ m/\s+(\d*\.\d+)%\s+\S+\s+\[\.\](.*)/;

      my $overhead = $1;
      my $funcname = $2;
      push @{$func_map_ref->{$funcname}}, $overhead;
      print $funcname, " " , join(", ", @{$func_map_ref->{$funcname}}), "\n" if 1 == $verbose;

      $count = $count + 1;
      if ($topn == $count) {
        last;
      }
    }

    return $sample_count;
}

sub dump_aggregated_profile {
  my $func_map_ref = shift @_;
  my %func_map = %$func_map_ref;

  print "\n\n";
  foreach my $key (keys %func_map) {
    my $value_list = $func_map{$key};
    my $len = scalar(@$value_list);
    print $key, "($len): " , join(", ", @$value_list), "\n\n";
  }
}

sub collect_perf_record_files {
    print "collect perf record files \n" if $verbose == 1;
    my @prof_files;
    my $cwd = getcwd();

    find ( sub  {
      return unless -f;       #Must be a file
      return unless /prof\.data\.\d+$/;  #Must end with `.pl` suffix
      #print $File::Find::name, "\n";
      push @prof_files, $File::Find::name;
      #print scalar(@prof_files), "\n";
                }, $cwd);
    return \@prof_files;
}

sub execute {
  my $args = shift @_;
  system("$args");
}
