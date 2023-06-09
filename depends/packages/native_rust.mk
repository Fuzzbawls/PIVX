package=native_rust
$(package)_version=1.60.0
$(package)_download_path=https://depends.pivx.org
$(package)_file_name_x86_64_linux=rust-$($(package)_version)-x86_64-unknown-linux-gnu.tar.xz
$(package)_sha256_hash_x86_64_linux=132c9bd63fbe219774992196005eb478e0e268e3ce6fc344ee3201579652bff9
$(package)_file_name_arm_linux=rust-$($(package)_version)-arm-unknown-linux-gnueabihf.tar.xz
$(package)_sha256_hash_arm_linux=4076345ec13ac9fcc721c34f1400b80605fcd57d9dd87196bad27bc70fd13a9a
$(package)_file_name_armv7l_linux=rust-$($(package)_version)-armv7-unknown-linux-gnueabihf.tar.xz
$(package)_sha256_hash_armv7l_linux=4076345ec13ac9fcc721c34f1400b80605fcd57d9dd87196bad27bc70fd13a9a
$(package)_file_name_aarch64_linux=rust-$($(package)_version)-aarch64-unknown-linux-gnu.tar.xz
$(package)_sha256_hash_aarch64_linux=604f537eae89f96c2377e12df609a70e26ebea7169e1bb8b2fec20f1ee288c0a
$(package)_file_name_x86_64_darwin=rust-$($(package)_version)-x86_64-apple-darwin.tar.xz
$(package)_sha256_hash_x86_64_darwin=5be9bfc9b3d4f170bc9fd44815179ca58fd8614a41e5be2e2369970b4286f004
$(package)_file_name_aarch64_darwin=rust-$($(package)_version)-aarch64-apple-darwin.tar.xz
$(package)_sha256_hash_aarch64_darwin=e52314376d5258f3fb3ec6b9e0164bfca1c15ed276bd0d772e5392ea8531afe4
$(package)_file_name_x86_64_freebsd=rust-$($(package)_version)-x86_64-unknown-freebsd.tar.xz
$(package)_sha256_hash_x86_64_freebsd=b5a96a9eb960bbfe527dba5549067102849fa80daabf524d367455c7b80232e1

# Mapping from GCC canonical hosts to Rust targets
# If a mapping is not present, we assume they are identical, unless $host_os is
# "darwin", in which case we assume x86_64-apple-darwin.
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu
$(package)_rust_target_i686-pc-linux-gnu=i686-unknown-linux-gnu
$(package)_rust_target_riscv64-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_riscv64-unknown-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_x86_64-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_x86_64-pc-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_armv7l-unknown-linux-gnueabihf=arm-unknown-linux-gnueabihf

# Mapping from Rust targets to SHA-256 hashes
$(package)_rust_std_sha256_hash_arm-unknown-linux-gnueabihf=b9dd30d6b9e7441bade7f1dcb96c37d194e9b818cf4f3be51558552564455267
$(package)_rust_std_sha256_hash_aarch64-unknown-linux-gnu=fa90bb7396fcb5ff78bb06d26c2ec7221308c400c31e3fc2f9ad09c7fa3098e1
$(package)_rust_std_sha256_hash_i686-unknown-linux-gnu=b3d0b4004bd14bb1197ae4c5403f008b34c7582386947b705cafad569be46656
$(package)_rust_std_sha256_hash_x86_64-unknown-linux-gnu=c5eade261a16cd7fbceae5c0fb77e5e0f1e521aa5941f8fb510b0a5fca7700cf
$(package)_rust_std_sha256_hash_riscv64gc-unknown-linux-gnu=2ca6b311ed6d5af2ac3f318461dfa26a8affaf999b590c8dd1d7b9fb621af9e6
$(package)_rust_std_sha256_hash_x86_64-apple-darwin=7eb4e17c03114e1a5860de096e3413e9eed70413cf63b2d541492b1c9f8c77c1
$(package)_rust_std_sha256_hash_aarch64-apple-darwin=365ab4623bbfd00d78390364748e2b4be547e068c1ec575226246694131cbe44
$(package)_rust_std_sha256_hash_x86_64-pc-windows-gnu=af6ffb5e1d8ce1cab7de8eb97aaf8dadd597dba468c2192edc0dcf2dc34c62c4

define rust_target
$(if $($(1)_rust_target_$(2)),$($(1)_rust_target_$(2)),$(if $(findstring darwin,$(3)),$(if $(findstring aarch64,$(host_arch)),aarch64-apple-darwin,x86_64-apple-darwin),$(2)))
endef

ifneq ($(canonical_host),$(build))
$(package)_rust_target=$(call rust_target,$(package),$(canonical_host),$(host_os))
$(package)_exact_file_name=rust-std-$($(package)_version)-$($(package)_rust_target).tar.xz
$(package)_exact_sha256_hash=$($(package)_rust_std_sha256_hash_$($(package)_rust_target))
$(package)_build_subdir=buildos
$(package)_extra_sources=$($(package)_file_name_$(build_arch)_$(build_os))

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_arch)_$(build_os)),$($(package)_file_name_$(build_arch)_$(build_os)),$($(package)_sha256_hash_$(build_arch)_$(build_os)))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_sha256_hash_$(build_arch)_$(build_os))  $($(package)_source_dir)/$($(package)_file_name_$(build_arch)_$(build_os))" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir $(canonical_host) && \
  tar --strip-components=1 -xf $($(package)_source) -C $(canonical_host) && \
  mkdir buildos && \
  tar --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_arch)_$(build_os)) -C buildos
endef

define $(package)_stage_cmds
  bash ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(build_prefix) --disable-ldconfig && \
  ../$(canonical_host)/install.sh --without=rust-docs --destdir=$($(package)_staging_dir) --prefix=$(build_prefix) --disable-ldconfig
endef
else

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_arch)_$(build_os)),$($(package)_file_name_$(build_arch)_$(build_os)),$($(package)_sha256_hash_$(build_arch)_$(build_os)))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash_$(build_arch)_$(build_os))  $($(package)_source_dir)/$($(package)_file_name_$(build_arch)_$(build_os))" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir $(canonical_host) && \
  tar --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_arch)_$(build_os)) -C $(canonical_host)
endef

define $(package)_stage_cmds
  bash ./$(canonical_host)/install.sh --without=rust-docs --destdir=$($(package)_staging_dir) --prefix=$(build_prefix) --disable-ldconfig
endef
endif
