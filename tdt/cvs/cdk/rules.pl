#!/usr/bin/perl
use strict;
use warnings;
use IO::File;

sub load ($$$);

sub load ($$$)
{
  my ( $filename, $package, $ignore ) = @_;

  my $fh = new IO::File $filename;

  if ( ! $fh )
  {
    return undef if $ignore;
    die "can't open $filename\n";
  }

  while ( <$fh> )
  {
    if ( ! m/^#/ )
    {
      chomp;
      my @rule = split ( /;/, $_ );

      next if not defined $rule[0];
      return @rule if $rule[0] eq $package;

      next if not defined $rule[1];
      my @ret;
      @ret = load ( $rule[1], $package, 0 ) if $rule[0] eq ">>>";
      @ret = load ( $rule[1], $package, 1 ) if $rule[0] eq ">>?";
      return @ret if @ret;
    }
  }

  die "can't find package $package";
}

sub process_make_depends (@)
{
  my $output;

  @_ = split ( /:/, $_[2] );

  foreach ( @_ )
  {
    if ( $_ =~ m#\.deb\.diff\.(bz2|gz)$# )
    {
      $output .= "Patches/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.(diff|tar)\.(bz2|gz)$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.(bz2|gz)$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.(config|diff|patch)$# )
    {
      $output .= "Patches/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.exe$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.tgz$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.zip$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.src\.rpm$# )
    {
      $output .= "Archive/" . $_ . " ";
    }
    elsif ( $_ =~ m#\.cvs# )
    {
      $output .= "Archive/". $_ . " ";
    }
    elsif ( $_ =~ m#\.svn# )
    {
      $output .= "\\\$(archivedir)/". $_ . " ";
    }
    elsif ($_ =~ m#\.git# )
    {
      $output .= "\\\$(archivedir)/". $_ . " ";
    }
    else
    {
      die "can't recognize type of archive " . $_;
    }
  }

  return "\"$output\"";
}

sub process_make_dir (@)
{
  return $_[1];
}

sub process_make_prepare (@)
{
  shift;
  my $dir = shift;
  shift;

  my $output = "( rm -rf " . $dir . " || /bin/true )";

  foreach ( @_ )
  {
    local @_;
    @_ = split ( /:/, $_ );

    if ( $output ne "" )
    {
      $output .= " && ";
    }

    if ( $_[0] eq "extract" )
    {
      if ( $_[1] =~ m#\.tar\.bz2$# )
      {
	$output .= "bunzip2 -cd Archive/" . $_[1] . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.tar\.gz$# )
      {
	$output .= "gunzip -cd Archive/" . $_[1] . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.tgz$# )
      {
	$output .= "gunzip -cd Archive/" . $_[1] . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.exe$# )
      {
	$output .= "cabextract Archive/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.zip$# )
      {
	$output .= "unzip -d $_[2] Archive/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.src\.rpm$# )
      {
	$output .= "rpm \${DRPM} -Uhv  Archive/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.cvs$# )
      {
	$_[1] =~ s/\.cvs//;
	$output .= "cp -a Archive/" . $_[1] . " . && mv " . $_[1] . " " . $dir;
      }
      else
      {
	die "can't recognize type of archive " . $_[1];
      }
    }
    elsif ( $_[0] eq "dirextract" )
    {
      $output .= "( mkdir " . $dir . " || /bin/true ) && ";
      $output .= "( cd " . $dir . "; ";

      if ( $_[1] =~ m#\.tar\.bz2$# )
      {
	$output .= "bunzip2 -cd ../Archive/" . $_[1] . " | tar -x";
      }
      elsif ( $_[1] =~ m#\.tar\.gz$# )
      {
	$output .= "gunzip -cd ../Archive/" . $_[1] . " | tar -x";
      }
      elsif ( $_[1] =~ m#\.exe$# )
      {
	$output .= "cabextract ../Archive/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.zip$# )
      {
        $output .= "unzip $_[2] \\\$(archivedir)/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.src\.rpm$# )
      {
        $output .= "rpm \${DRPM} -Uhv  \\\$(archivedir)/" . $_[1];
      }
      elsif ( $_[1] =~ m#\.cvs# )
      {
        my $target = $dir;
        if ( @_ > 2 )
        {
          $target = $_[2] 
        }
        $output .= "cp -a \\\$(archivedir)/" . $_[1] . " " . $target;
      }
      elsif ( $_[1] =~ m#\.svn# )
      {
        my $target = $dir;
        if ( @_ > 2 )
        {
          $target = $_[2] 
        }
        $output .= "cp -a \\\$(archivedir)/" . $_[1] . " " . $target;
      }
      elsif ( $_[1] =~ m#\.git# )
      {
        my $target = $dir;
        if ( @_ > 2 )
        {
          $target = $_[2] 
        }
        $output .= "(rm -rf " . $target . "; cp -a \\\$(archivedir)/" . $_[1] . " " . $target . ")";
      }
      else
      {
	die "can't recognize type of archive " . $_[1];
      }

      $output .= " )";
    }
    elsif ( $_[0] =~ m/patch(time)?(-(\d+))?/ )
    {
      $_ = "-p1 ";
      $_ = "-p$3 " if defined $3;
      $_ .= "-Z " if defined $1;
      if ( $_[1] =~ m#\.bz2$# )
      {
	$output .= "( cd " . $dir . "; bunzip2 -cd ../Archive/" . $_[1] . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.deb\.diff\.gz$# )
      {
	$output .= "( cd " . $dir . "; gunzip -cd ../Patches/" . $_[1] . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.gz$# )
      {
	$output .= "( cd " . $dir . "; gunzip -cd ../Archive/" . $_[1] . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.spec\.diff$# )
      {
	$output .= "( cd SPECS && patch $_ < ../Patches/" . $_[1] . " )";
      }
      else
      {
	$output .= "( cd " . $dir . "; patch $_ < ../Patches/" . $_[1] . " )";
      }
    }
    elsif ( $_[0] eq "rpmbuild" )
    {
      $output .= "rpmbuild \${DRPMBUILD} -bb -v --clean --target=sh4-linux SPECS/stm-" . $_[1] . ".spec ";
    }
    elsif ( $_[0] eq "move" )
    {
      $output .= "mv " . $_[1] . " " . $_[2];
    }
    elsif ( $_[0] eq "remove" )
    {
      $output .= "( rm -rf " . $_[1] . " || /bin/true )";
    }
    elsif ( $_[0] eq "link" )
    {
      $output .= "( ln -sf " . $_[1] . " " . $_[2] . " || /bin/true )";
    }
    elsif ( $_[0] eq "dircreate" )
    {
      $output .= "( mkdir -p $dir )";
    }
    else
    {
      die "can't recognize @_";
    }
  }

  return "\"$output\"";
}

sub process_make_version (@)
{
  return $_[0];
}

sub process_make ($$$)
{
  my $package = $_[0];
  my @rules = @{$_[1]};
  my $arg = @{$_[2]}[0];
  my $output = "";

  my %args =
  (
    depends => \&process_make_depends,
    dir => \&process_make_dir,
    prepare => \&process_make_prepare,
    version => \&process_make_version,
  );

  if ( $arg eq "cdkoutput" )
  {
    foreach ( sort keys %args )
    {
      ( my $tmp = $_ ) =~ y/a-z/A-Z/;
      $output .= $tmp . "_" . $package . "=" . &{$args{$_}} (@rules) . "\n";
    }
  }
  else
  {
    die "can't recognize $arg" if not $args{$arg};

    $output = &{$args{$arg}} (@rules);
  }

  return $output;
}

sub process_install_rule ($)
{
  my $rule = shift;
  @_ = split ( /:/, $rule );
  $_ = shift @_;

  my $output = "";

  if ( $_ eq "make" )
  {
    $output .= "\$\(MAKE\) " . join " ", @_;
  }
  elsif ( $_ eq "install" )
  {
    $output .= "\$\(INSTALL\) " . join " ", @_;
  }
  elsif ( $_ eq "rpminstall" )
  {
    $output .= "rpm \${DRPM} --ignorearch -Uhv RPMS/sh4/" . join " ", @_;
  }
  elsif ( $_ eq "shellconfigadd" )
  {
    $output .= "export HCTDINST \&\& HOST/bin/target-shellconfig --add " . join " ", @_;
  }
  elsif ( $_ eq "initdconfigadd" )
  {
    $output .= "export HCTDINST \&\& HOST/bin/target-initdconfig --add " . join " ", @_;
  }
  elsif ( $_ eq "move" )
  {
    $output .= "mv " . join " ", @_;
  }
  elsif ( $_ eq "remove" )
  {
    $output .= "rm -rf " . join " ", @_;
  }
  elsif ( $_ eq "mkdir" )
  {
    $output .= "mkdir -p " . join " ", @_;
  }
  elsif ( $_ eq "link" )
  {
    $output .= "ln -sf " . join " ", @_;
  }
  elsif ( $_ eq "archive" )
  {
    $output .= "TARGETNAME-ar cru " . join " ", @_;
  }
  elsif ( $_ =~ m/^rewrite-(libtool|pkgconfig|dependency)/ )
  {
    $output .= "perl -pi -e \"s,^libdir=.*\$\$,libdir='TARGET/usr/lib',\" ". join " ", @_ if $1 eq "libtool";
    $output .= "perl -pi -e \"s, /usr/lib, TARGET/usr/lib,g if /^dependency_libs/\"  ". join " ", @_ if $1 eq "dependency";
    $output .= "perl -pi -e \"s,^prefix=.*\$\$,prefix=TARGET/usr,\" " . join " ", @_ if $1 eq "pkgconfig";
  }
  else
  {
    die "can't recognize rule \"$rule\"";
  }

  return $output;
}

sub process_install ($$$)
{
  my @rules = @{$_[1]};
  my $output = "";

  foreach ( @rules )
  {
    $output .= " && " if $output;
    $output .= process_install_rule ($_);
  }

  return $output;
}

my %ruletypes =
(
  make => { process => \&process_make, further_args => 1 },
  install => { process => \&process_install },
  uninstall => { process => \&process_uninstall },
);

die "please specify a rule type, filename and a package" if $#ARGV < 2;

my $ruletype = shift;
my $filename = shift;
my $package = shift;

die "can't determine rule type" if not $ruletypes{$ruletype};
die "rule type needs further args" if $ruletypes{$ruletype}->{further_args} and $#ARGV + 1 < $ruletypes{$ruletype}->{further_args};

my @rules = load ( $filename, $package, 0 );
shift @rules;

my $output = &{$ruletypes{$ruletype}->{process}} ($package, \@rules, \@ARGV);

if ( $output )
{
  $output =~ s#TARGETNAME#\$\(target\)#g;
  $output =~ s#TARGETS#\$\(prefix\)\/\$\*cdkroot#g;
  $output =~ s#TARGET#\$\(targetprefix\)#g;
  $output =~ s#HCTDINST#HHL\_CROSS\_TARGET\_DIR\=\$\(prefix\)\/\$\*cdkroot#g;
  $output =~ s#HCTDUNINST#HHL\_CROSS\_TARGET\_DIR\=\$\(targetprefix\)#g;
  $output =~ s#HOST#\$\(hostprefix\)#g;
  $output =~ s#BUILD#\$\(buildprefix\)#g;
  print $output . "\n";
}

