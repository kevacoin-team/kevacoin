package=native_cdrkit
$(package)_version=1.1.11
$(package)_download_path=http://distro.ibiblio.org/fatdog/source/600/c
$(package)_file_name=cdrkit-$($(package)_version).tar.bz2
$(package)_sha256_hash=b50d64c214a65b1a79afe3a964c691931a4233e2ba605d793eb85d0ac3652564
$(package)_patches=cdrkit-deterministic.patch

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/cdrkit-deterministic.patch
endef

GXX_VERSION := $(shell g++ -dumpversion)

ifneq ($(shell expr $(GXX_VERSION) \>= 10), 0)
  CFLAGS += -fcommon
  CXXFLAGS += -fcommon
endif

define $(package)_config_cmds
  cmake -DCMAKE_INSTALL_PREFIX=$(build_prefix) \
        -DCMAKE_C_FLAGS="$(CFLAGS)" \
        -DCMAKE_CXX_FLAGS="$(CXXFLAGS)"
endef

define $(package)_build_cmds
  $(MAKE) genisoimage
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) -C genisoimage install
endef

define $(package)_postprocess_cmds
  rm bin/isovfy bin/isoinfo bin/isodump bin/isodebug bin/devdump
endef
