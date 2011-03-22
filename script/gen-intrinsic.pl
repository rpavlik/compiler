#!/usr/bin/perl
use Data::Dumper;

sub fprint{
        my($filename,$content)=@_;
        return if not ($content);
        open(FILE, ">".$filename) or die "can't open file: $filename\n";
        print FILE $content;
        close FILE;
}

sub copy_to{
	my($path,$filename,$path2)=@_;
	return if not ($filename);
	if($path2 eq ""){
	  system("mv -f $filename $path") ;
	}else{
	  system("cp -f $filename $path");
	  system("mv -f $filename $path2");
	}

}

sub cartesian {		#cartesian product, the perl way ]:->
    my @C=[];
    foreach(reverse @_){
        my @A=@$_;
        @C=map{my $n=$_;map{[$n,@$_]} @C} @A;
    }
    return @C;
}

sub builtin_type { ##this define the builtin type of each parameter
    my($arg)=@_;
    if($arg=~/v32f4/){
      if($arg=~/mem/){
        return "pv8sf_type_node";
      }
      return "V8SF_type_node";
    }
    elsif($arg=~/v32f8/){
	if($arg=~/mem/){
	  return "pv4df_type_node";
	}
      return "V4DF_type_node";
    }
    elsif($arg=~/v32i4/){
	if($arg=~/mem/){
	  return "pv8si_type_node";
	}
      return "V8SI_type_node";
    }
    elsif($arg=~/v32i8/){
	if($arg=~/mem/){
	  return "pv4di_type_node";
	}
      return "V4DI_type_node";
    }
    elsif($arg=~/v16f4/){
	if($arg=~/mem/){
	  return "pv4sf_type_node";
	}
      return "V4SF_type_node";
    }
    elsif($arg=~/v16f8/){
	if($arg=~/mem/){
	  return "pv2df_type_node";
	}
      return "V2DF_type_node";
    }
    elsif($arg=~/v16i4/){
	if($arg=~/mem/){
	  return "pv4si_type_node";
	}
      return "V4SI_type_node";
    }
    elsif($arg=~/v16i8/){
	if($arg=~/mem/){
	  return "pv2di_type_node";
	}
 	return "V2DI_type_node";
    }
    elsif($arg=~/simm8/){
    	return "integer_type_node";
    }
    elsif($arg=~/memdb/){
    	return "pdouble_type_node";
    }
    elsif($arg=~/memfl/){
    	return "pfloat_type_node";
    }
    else{
      die "don't support at the moment: ".$arg."\n";
    }
}


#pulling things out of envytools and converting them directly to isa_* interface results in fugly names, but at least it's fast as hell.

#my @mem=[["base64"],["offset32"],[["base64"],["offset64"],["uimm8"],["simm32"]],[["offset64"],["uimm8"],["simm32"]];
#my @mem =["base64","offset32"];
#TODO builtin  __builtin_ia32_haddpd256
my @ov32f8_v32f8_v32f8=["VADDPD256 __builtin_ia32_addpd256","VADDSUBPD256 __builtin_ia32_addsubpd256","VHADDPD256 __builtin_ia32_haddpd256","VSUBPD256 __builtin_ia32_hsubpd256","VHSUBPD256","VMULPD256 __builtin_ia32_mulpd256","VDIVPD256 __builtin_ia32_divpd256","VANDPD256 __builtin_ia32_andnpd256","VANDNPD256 __builtin_ia32_andpd256","VORPD256","VXORPD256","VMAXPD256 __builtin_ia32_maxpd256","VMINPD256 __builtin_ia32_minpd256"];
my @ov32f4_v32f4_v32f4=["VADDPS256 __builtin_ia32_addps256","VADDSUBPS256 __builtin_ia32_addsubps256","VHADDPS __builtin_ia32_haddps256","VSUBPS256","VHSUBPS256 __builtin_ia32_hsubps256","VMULPS256 __builtin_ia32_mulps256","VDIVPS256 __builtin_ia32_divps256","VANDPS256 __builtin_ia32_andps256","VANDNPS256 __builtin_ia32_andnps256","VORPS256","VXORPS256","VMAXPS256 __builtin_ia32_maxps256","VMINPS256 __builtin_ia32_minps256"];
my @ov32f4_v32f4_v32f4_simm8=["VDPPS __builtin_ia32_dpps256","VBLENDPS __builtin_ia32_blendps256","VBLENDVPS256 __builtin_ia32_blendvps256","VCMPPS"];
my @ov16f4_v16f4_v16f4_simm8=["VCMPPS","VCMPSS"];
my @ov32f8_v32f8_v32f8_simm8=["VBLENDPD __builtin_ia32_blendpd256","VBLENDVPD256 __builtin_ia32_blendvpd256","VCMPPD"];
my @ov16f8_v16f8_v16f8_simm8=["VCMPPD","VCMPSD"];
my @ov32f8_v32f8=["VSQRTPD"];
my @ov32f4_v32f4=["VSQRTPS","VRSQRTPS","VRCPPS"];
my @ov32f8_v16i4=["VCVTDQ2PD"];
my @ov32f4_v32i4=["VCVTDQ2PS"];
my @ov16i4_v32f8=["VCVTPD2DQ","VCVTTPD2DQ"];
my @ov32i4_v32f4=["VCVTPS2DQ"];
my @ov16f4_v32f8=["VCVTPD2PS"];
my @ov32f8_v16f4=["VCVTPS2PD"];
my @ov16i4_v32f4=["VCVTTPS2DQ"];

my @ov32f8_v16f8mem=["VBROADCAST128"];
my @ov32f4_v16f4mem=["VBROADCAST128"];
my @ov32f8_dbmem=["VBROADCASTSD","VMOVAPD","VMOVUPD"];
my @ov32f4_flmem=["VBROADCASTSS","VMOVAPS","VMOVUPS"];
my @ov16f4_flmem=["VROADCASTSS"];
my @ov32i4_v32i4mem=["VMOVDQA"];
my @ov32f8_dbmem_v32i4=["VMASKMOVPD"];
my @ov16f8_dbmem_v16i4=["VMASKMOVPD"];
my @ov32f4_flmem_v32i4=["VMASKMOVPS"];
my @ov16f4_flmem_v16i4=["VMASKMOVPS"];
#my @ov32f8_dbmem=["VMOVAPD"];

##TODO _mm256_store_pd

my @ops=(
		#[@vaddp,["f128","f256"], ["OPS"],["ofloat"],["float"],@float_mem],
		[["NAME"],@ov32f8_v32f8_v32f8, ["OPS"],["V32F8ov32f8"],["v32f8"],["v32f8"]],
		[["NAME"],@ov32f4_v32f4_v32f4, ["OPS"],["V32F4ov32f4"],["v32f4"],["v32f4"]],
		[["NAME"],@ov32f4_v32f4_v32f4_simm8,["OPS"],["V32F4ov32f4"],["v32f4"],["v32f4"],["simm8"]],
		[["NAME"],@ov32f8_v32f8_v32f8_simm8,["OPS"],["V32F8ov32f8"],["v32f8"],["v32f8"],["simm8"]],
		[["NAME"],@ov32f4_v32f4, ["OPS"],["V32F8ov32f4"],["V32F4ov32f4"]],
		[["NAME"],@ov32f8_v32f8, ["OPS"],["V32F8ov32f8"],["V32F8ov32f8"]],
		[["NAME"],@ov16f8_v16f8_v16f8_simm8,["OPS"],["V16F8ov16f8"],["v16f8"],["v16f8"],["simm8"]],
		[["NAME"],@ov16f4_v16f4_v16f4_simm8,["OPS"],["V16F4ov16f4"],["v16f4"],["v16f4"],["simm8"]],
		[["NAME"],@ov32f8_v16i4, ["OPS"],["V32F8ov32f8"],["v16i4"]],
		[["NAME"],@ov32f4_v32i4, ["OPS"],["V32F4ov32f4"],["v32i4"]],
		[["NAME"],@ov16i4_v32f8, ["OPS"],["V16I4ov16i4"],["v32f8"]],
		[["NAME"],@ov32i4_v32f4, ["OPS"],["V32I4ov32i4"],["v32f4"]],
		[["NAME"],@ov16f4_v32f8, ["OPS"],["V16F4ov16f4"],["v32f8"]],
		[["NAME"],@ov32f8_v16f4, ["OPS"],["V32F8ov32f8"],["v16f4"]],
		[["NAME"],@ov32f8_v16f8mem, ["OPS"],["V32F8ov32f8"],["memv16f8"]],
		[["NAME"],@ov32f4_v16f4mem, ["OPS"],["V32F4ov32f4"],["memv16f4"]],
		[["NAME"],@ov32f8_dbmem, ["OPS"],["V32F8ov32f8"],["memdb"]],
		[["NAME"],@ov32f4_flmem, ["OPS"],["V32F4ov32f4"],["memfl"]],
		[["NAME"],@ov16f4_flmem, ["OPS"],["V16F4ov16f4"],["memfl"]],
		[["NAME"],@ov32i4_v32i4mem, ["OPS"],["V32I4ov32i4"],["memv32i4"]],
		[["NAME"],@ov32f8_dbmem_v32i4, ["OPS"],["V32F8ov32f8"],["memdb"],["v32i4"]],
		[["NAME"],@ov16f8_dbmem_v16i4, ["OPS"],["V16F8ov16f8"],["memdb"],["v16i4"]],
		[["NAME"],@ov32f4_flmem_v32i4, ["OPS"],["V32F4ov32f4"],["memfl"],["v32i4"]],
		[["NAME"],@ov16f4_flmem_v16i4, ["OPS"],["V16F4ov16f4"],["memfl"],["v16i4"]],
		#[@ov32f8_dbmem, ["OPS"],["ov32f8"],["memdb"]],


	);
my @isa;
my @tops; 		#for isa pack and subset
my %isa_operands;
my %isa_print;
foreach $op (@ops){
	my @variants=cartesian(@$op);
	foreach(@variants){
		my $opstring;
		my @opname;
		my @operands;
		my $op_pattern;
		my $is_buildin=0;
		my $isop=0;
		foreach (@$_){
			if($_ eq "OPS"){
				$isop=1;
				$is_buildin=0;
			}
			elsif($_ eq "NAME"){
				$is_buildin=1;
				$isop=0;
			}
			elsif(not $_ eq ""){
				#$opstring.=$_."_";
				#push(@operands,$_) if $isop;
				if($isop){
				  $opstring.=$_."_";
				}
				push(@opname,$_) if $is_buildin;
				#$op_pattern.=$_."_" if $isop;
			}
		}
		#chop($op_pattern);
		chop($opstring);
		#my $topstring="TOP_".$opstring;
		if(scalar(@opname)>1){
		  die "OPNAME should be unique\n".@opname;
		}
		#push @isa,$opstring;
		#push @tops,$topstring;
		push @{$isa_operands{$opname[0]}},$opstring;
		#print "pushed "."opname ".$opname[0]." opstring". $opstring."\n";
		#push @{$isa_print{$op_pattern}},$topstring;
	}
}

my @debug_dup_isa;
my $di=0;

print "isa.cxx:\n";
#foreach(@isa){
#	$isa_isa_print.="\t\"".$_."\",\n";
#	$debug_dup_isa[$di++]=$_;
#}

#print "di = $di\n";

#my $dj;
#my $dk;
#for($dj=0;$dj<$di;$dj++){
	#print "dj=$dj di=$di\n";
#  for($dk=0;$dk<$di;$dk++){
#    if($dj!=$dk){
#      if($debug_dup_isa[$dk]=~/^$debug_dup_isa[$dj]$/i){
#	      print "shit met duplicate:\n".$debug_dup_isa[$dk]."   ".$debug_dup_isa[$dj]."   dk=$dk  dj=$dj\n";
#      }
#    }
#  }
  #print "shit ".$debug_dup_isa[$dj]."\n";
#}

#$isa_print="\nisa_operands.cxx:\n";
my $print_i386_c;
my $print_i386_h;
my $print_tree_c;
my $print_gspin_tree_h;
my $print_gspin_tree_cxx;
my $print_intrn_info_cxx;
my $print_wintrinsic_h;
my $print_wutil_cxx;
my $wgen_expr_cxx;
foreach (keys %isa_operands){
  #print "keys ".$_."\n";
  #foreach(@{$isa_operands{$_}}){
  #  print "\t\t".$_."\n";
    #}
  print "\t\t".${$isa_operands{$_}}[0]."\n";
  my $intrin_name;
  my $buildin_name;
  my @opnd;
  my @tmp;
  @tmp=split(" ",$_);
  $intrin_name=$tmp[0];
  $buildin_name=$tmp[1];
  if($buildin_name eq ""){
	  #die $intrin_name." doesn't get builtin"."\n";
	  next;
  }

  my @opnd=split("_", ${$isa_operands{$_}}[0]);

  my $i;
  my $ftype;
  for($i=0;$i<scalar(@opnd);$i++){
    $ftype.=builtin_type($opnd[$i]);
    $ftype.=", ";
  }
  $print_i386_c.="ftype = build_function_type_list(";
  $print_i386_c.=$ftype."NULL_TREE);\n";
  $print_i386_c.="def_builtin (MASK_AVX, \"".$buildin_name."\","."ftype,";
  $print_i386_c.="IX86_BUILTIN_".$intrin_name.");\n\n";

  $print_i386_h.="IX86_BUILTIN_".$intrin_name.",\n";

  $print_tree_c.="case IX86_BUILTIN_".$intrin_name.":";
  $print_tree_c.="return GSBI_IX86_BUILTIN_".$intrin_name.";\n";
  
  $print_gspin_tree_h.="GSBI_IX86_BUILTIN_".$intrin_name.",\n";

  my $intrn_ret;
  #$intrn_ret=return_type($opnd[0]);
  $intrn_ret=$opnd[0];
  my @pre_ret;
  @pre_ret=split("o",$intrn_ret);

  $print_intrn_info_cxx.="{\/\*".$intrin_name."\*\/\n";
  $print_intrn_info_cxx.="BYVAL, PURE, NO_SIDEEFFECTS, DOES_RETURN, NOT_ACTUAL, CGINTRINSIC,\n";
  $print_intrn_info_cxx.="IRETURN_".$pre_ret[0].",\"".$intrin_name."\"".",NULL, NULL},\n";
  ##TODO return type should modify accordingly.


  $print_wintrinsic_h.="INTRN_".$intrin_name.",\n";
  $print_wutil_cxx.="INTRN_".$intrin_name.",\t"."\""."INTRN_".$intrin_name."\",\n";
  $print_wgen_expr_cxx.="case "."GSBI_IX86_BUILTIN_".$intrin_name.":"."\n";
  $print_wgen_expr_cxx.="\*iopc = "."INTRN_".$intrin_name.";"."\n";
  $print_wgen_expr_cxx.="break;"."\n";
}

sub copy_all_file{
  copy_to('../GCC/gcc/config/i386/',"i386_avx.h");
  copy_to('../GCC/gcc/config/i386/',"i386_avx.c");
  copy_to('../GCC/gcc/',"tree_avx.c");
  copy_to('../GCC/libspin/',"gspin-tree_avx.h","../src/wgen/");
  copy_to('../src/common/com/',"intrn_info_avx.cxx");
  copy_to('../src/common/com/',"wintrinsic_avx.h");
  copy_to('../src/common/com',"wutil_avx.cxx");
  #copy_to('../src/wgen/',"gspin-tree_avx.cxx");
  copy_to('../src/wgen/',"wgen_expr_avx.cxx");
}

print "i386.c:\n";
print $print_i386_c;
fprint("i386_avx.c",$print_i386_c);
print "\n\n";
print "i364.h:\n";
print $print_i386_h;
fprint("i386_avx.h",$print_i386_h);
print "\n\n";
print "tree.c\n";
print $print_tree_c;
fprint("tree_avx.c",$print_tree_c);
print "\n\n";
print "gspin_tree.h\n";
print $print_gspin_tree_h;
fprint("gspin-tree_avx.h",$print_gspin_tree_h);
print "\n\n";
print "intrn_info.cxx\n";
print $print_intrn_info_cxx;
fprint("intrn_info_avx.cxx",$print_intrn_info_cxx);
print "\n\n";
print "wintrinsic.h\n";
fprint("wintrinsic_avx.h",$print_wintrinsic_h);
print $print_wintrinsic_h;
print "\n\n";
print "wutil.cxx\n";
print $print_wutil_cxx;
fprint("wutil_avx.cxx",$print_wutil_cxx);
print "\n\n";
print "wgen_expr.cxx\n";
fprint("wgen_expr_avx.cxx",$print_wgen_expr_cxx);
print $print_wgen_expr_cxx;
print "\n\n";

copy_all_file();
