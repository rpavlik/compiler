set(PATHSCALE_LNO_COMMON_SRCS
				"../com/cxx_template.cxx"
				"@ARCH@/lnotarget.cxx"
				"lnodriver.cxx"
				"lnopt_main.cxx"
				"lnoutils.cxx"
				"access_main.cxx"
				"access_vector.cxx"
				"soe.cxx"
				"mat_textra.cxx"
				"pad.cxx"
				"lu_mat_textra.cxx"
				"vs_textra.cxx"
				"frac.cxx"
				"fission.cxx"
				"fis_gthr.cxx"
				"fusion.cxx"
				"fiz_fuse.cxx"
				"inner_fission.cxx"
				"lnopt_hoistif.cxx"
				"vintr_fis.cxx"
				"ff_utils.cxx"
				"ff_pragmas.cxx"
				"name.cxx"
				"outer.cxx"
				"lno_bv.cxx"
				"scalar_expand.cxx"
				"can.cxx"
				"aequiv.cxx"
				"sclrze.cxx"
				"dead.cxx"
				"minvariant.cxx"
				"oinvar.cxx"
				"lno_split.cxx"
				"cse.cxx"
				"model.cxx"
				"cache_model.cxx"
				"dep.cxx"
				"reduc.cxx"
				"zmult.cxx"
				"snl_deps.cxx"
				"snl_dist.cxx"
				"snl_nest.cxx"
				"snl_test.cxx"
				"snl_trans.cxx"
				"snl_xbounds.cxx"
				"snl_utils.cxx"
				"snl_gen.cxx"
				"snl_inv.cxx"
				"cond.cxx"
				"prefetch.cxx"
				"pf_ref.cxx"
				"pf_loop.cxx"
				"lwn_util.cxx"
				"lno_scc.cxx"
				"debug.cxx"
				"lego_pragma.cxx"
				"lego_io.cxx"
				"lego_local.cxx"
				"lego_gen.cxx"
				"lego_util.cxx"
				"lego.cxx"
				"array_lower.cxx"
				"pf_manual.cxx"
				"forward.cxx"
				"ara.cxx"
				"ara_region.cxx"
				"ara_loop.cxx"
				"ara_live.cxx"
				"ara_utils.cxx"
				"reverse.cxx"
				"permute.cxx"
				"move.cxx"
				"tile.cxx"
				"lego_opts.cxx"
				"lego_affinity.cxx"
				"al_ref.cxx"
				"al_loop.cxx"
				"lego_ec.cxx"
				"array_bounds.cxx"
				"lego_skew.cxx"
				"small_trips.cxx"
				"sxlist.cxx"
				"sxlimit.cxx"
				"parallel.cxx"
				"if_info.cxx"
				"call_info.cxx"
				"parmodel.cxx"
				"sdlist.cxx"
				"doacross.cxx"
				"autod.cxx"
				"parids.cxx"
				"ifminmax.cxx"
				"split_tiles.cxx"
				"wind_down.cxx"
				"dnf.cxx"
				"cxx_queue.cxx"
				"shackle.cxx"
				"shackle_ifs.cxx"
				"ipa_lno_read.cxx"
				"ipa_lno_reshape.cxx"
				"mem_sim.cxx"
				"shackle_mem.cxx"
				"eliminate.cxx"
				"ipa_lno_cost.cxx"
				"cross_snl.cxx"
				"cross_cache.cxx"
                                "lno_trace.cxx"
				"lx_prompf_anl.cxx"
                                "access_vector.cxx"
                                "../com/reshape.cxx")

set(PATHSCALE_LNO_X8664_SRCS "simd.cxx")

set(PATHSCALE_LNO_MIPS_SRCS "simd_mips.cxx")

ADD_DEFINITIONS(
	-DLNO
	-DBACK_END
	-D_NEW_SYMTAB
	-DMONGOOSE_BE)

path64_set_multiarch_sources(lno_sources COMMON ${PATHSCALE_LNO_COMMON_SRCS})
path64_set_multiarch_sources(lno_sources x8664 ${PATHSCALE_LNO_COMMON_SRCS} ${PATHSCALE_LNO_X8664_SRCS})
path64_set_multiarch_sources(lno_sources mips ${PATHSCALE_LNO_COMMON_SRCS} ${PATHSCALE_LNO_MIPS_SRCS})
path64_add_multiarch_library(lno STATIC lno_sources)
path64_set_multiarch_property(lno PREFIX "")
path64_multiarch_link_multiarch_libraries(lno wopt prompf_anl comutil)
path64_add_multiarch_dependencies(lno generate_preg_list generate_topcode)
path64_set_multiarch_property(lno COMPILE_FLAGS
                              "-DSHARED_BUILD -I${PATHSCALE_BINARY_DIR}/common/targ_info/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../../common/com/@ARCH@")
path64_install_multiarch(lno)
path64_add_dependencies_from_multiarch(compiler-stage lno)

