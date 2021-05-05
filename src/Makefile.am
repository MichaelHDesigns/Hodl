DIST_SUBDIRS = secp256k1
AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS)


if EMBEDDED_LEVELDB
LEVELDB_CPPFLAGS += -I$(srcdir)/leveldb/include
LEVELDB_CPPFLAGS += -I$(srcdir)/leveldb/helpers/memenv
LIBLEVELDB += $(builddir)/leveldb/libleveldb.a
LIBMEMENV  += $(builddir)/leveldb/libmemenv.a

# NOTE: This dependency is not strictly necessary, but without it make may try to build both in parallel, which breaks the LevelDB build system in a race
$(LIBLEVELDB): $(LIBMEMENV)

$(LIBLEVELDB) $(LIBMEMENV):
	@echo "Building LevelDB ..." && $(MAKE) -C $(@D) $(@F) CXX="$(CXX)" \
	  CC="$(CC)" PLATFORM=$(TARGET_OS) AR="$(AR)" $(LEVELDB_TARGET_FLAGS) \
          OPT="$(CXXFLAGS) $(CPPFLAGS) -D__STDC_LIMIT_MACROS -std=c++11"
endif

HODLCASH_CONFIG_INCLUDES=-I$(builddir)/config
HODLCASH_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

HODLCASH_INCLUDES += -I$(srcdir)/secp256k1/include

LIBHODLCASH_SERVER=libhodlcash_server.a
LIBHODLCASH_WALLET=libhodlcash_wallet.a
LIBHODLCASH_COMMON=libhodlcash_common.a
LIBHODLCASH_CLI=libhodlcash_cli.a
LIBHODLCASH_UTIL=libhodlcash_util.a
LIBHODLCASH_CRYPTO=crypto/libhodlcash_crypto.a
LIBHODLCASH_CRYPTO_X86=libhodlcash_crypto_x86.a
LIBHODLCASH_UNIVALUE=univalue/libhodlcash_univalue.a
LIBHODLCASHQT=qt/libhodlcashqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES = \
  crypto/libhodlcash_crypto.a \
  libhodlcash_util.a \
  libhodlcash_common.a \
  libhodlcash_crypto_x86.a \
  univalue/libhodlcash_univalue.a \
  libhodlcash_server.a \
  libhodlcash_cli.a
if ENABLE_WALLET
HODLCASH_INCLUDES += $(BDB_CPPFLAGS)
EXTRA_LIBRARIES += libhodlcash_wallet.a
endif

if BUILD_HODLCASH_LIBS
lib_LTLIBRARIES = libhodlcashconsensus.la
LIBHODLCASH_CONSENSUS=libhodlcashconsensus.la
else
LIBHODLCASH_CONSENSUS=
endif

bin_PROGRAMS =
TESTS =

if BUILD_HODLCASHD
  bin_PROGRAMS += HodlCashd
endif

if BUILD_HODLCASH_UTILS
  bin_PROGRAMS += HodlCash-cli HodlCash-tx
endif

.PHONY: FORCE
# HodlCash core #
HODLCASH_CORE_H = \
  addrman.h \
  aligned_malloc.h \
  aes.h \
  alert.h \
  amount.h \
  arith_uint256.h \
  base58.h \
  bloom.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/params.h \
  consensus/validation.h \
  core_io.h \
  eccryptoverify.h \
  ecwrapper.h \
  hash.h \
  init.h \
  key.h \
  keystore.h \
  leveldbwrapper.h \
  limitedmap.h \
  main.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  mruset.h \
  net.h \
  netbase.h \
  noui.h \
  patternsearch.h \
  policy/fees.h \
  pow.h \
  primitives/block.h \
  primitives/transaction.h \
  protocol.h \
  pubkey.h \
  random.h \
  reverselock.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  scheduler.h \
  script/interpreter.h \
  script/script.h \
  script/script_error.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  sha512.h \
  serialize.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/pagelocker.h \
  sync.h \
  threadsafety.h \
  timedata.h \
  tinyformat.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  undo.h \
  util.h \
  utilmoneystr.h \
  utilstrencodings.h \
  utiltime.h \
  validationinterface.h \
  version.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/wallet.h \
  wallet/wallet_ismine.h \
  wallet/walletdb.h \
  aescache.h

JSON_H = \
  json/json_spirit.h \
  json/json_spirit_error_position.h \
  json/json_spirit_reader.h \
  json/json_spirit_reader_template.h \
  json/json_spirit_stream_reader.h \
  json/json_spirit_utils.h \
  json/json_spirit_value.h \
  json/json_spirit_writer.h \
  json/json_spirit_writer_template.h

obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libhodlcash_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between HodlCashd and HodlCash-qt
libhodlcash_server_a_CPPFLAGS = $(HODLCASH_INCLUDES) $(MINIUPNPC_CPPFLAGS)
libhodlcash_server_a_SOURCES = \
  addrman.cpp \
  alert.cpp \
  bloom.cpp \
  chain.cpp \
  checkpoints.cpp \
  init.cpp \
  main.cpp \
  merkleblock.cpp \
  miner.cpp \
  net.cpp \
  noui.cpp \
  patternsearch.cpp \
  policy/fees.cpp \
  pow.cpp \
  rest.cpp \
  rpcblockchain.cpp \
  rpcmining.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
  rpcserver.cpp \
  script/sigcache.cpp \
  timedata.cpp \
  txmempool.cpp \
  validationinterface.cpp \
  $(JSON_H) \
  $(HODLCASH_CORE_H)

# wallet: shared between HodlCashd and HodlCash-qt, but only linked
# when wallet enabled
libhodlcash_wallet_a_CPPFLAGS = $(HODLCASH_INCLUDES)
libhodlcash_wallet_a_SOURCES = \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/wallet_ismine.cpp \
  wallet/walletdb.cpp \
  $(HODLCASH_CORE_H)

# crypto primitives library
crypto_libhodlcash_crypto_a_CPPFLAGS = $(HODLCASH_CONFIG_INCLUDES)
crypto_libhodlcash_crypto_a_SOURCES = \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h

# univalue JSON library
univalue_libhodlcash_univalue_a_SOURCES = \
  univalue/univalue.cpp \
  univalue/univalue.h \
  univalue/univalue_escapes.h \
  univalue/univalue_read.cpp \
  univalue/univalue_write.cpp

libhodlcash_crypto_x86_a_CPPFLAGS = $(HODLCASH_INCLUDES) $(CRYPTO_X86_CXXFLAGS) $(CRYPTO_AES_CXXFLAGS) $(CRYPTO_AVX2_CXXFLAGS)
libhodlcash_crypto_x86_a_SOURCES = \
  aes.cpp \
  sha512.cpp

# common: shared between HodlCashd, and HodlCash-qt and non-server tools
libhodlcash_common_a_CPPFLAGS = $(HODLCASH_INCLUDES)
libhodlcash_common_a_SOURCES = \
  amount.cpp \
  arith_uint256.cpp \
  base58.cpp \
  chainparams.cpp \
  txdb.cpp \
  leveldbwrapper.cpp \
  aescache.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  eccryptoverify.cpp \
  ecwrapper.cpp \
  hash.cpp \
  key.cpp \
  keystore.cpp \
  netbase.cpp \
  patternsearch.cpp \
  primitives/block.cpp \
  primitives/transaction.cpp \
  protocol.cpp \
  pubkey.cpp \
  scheduler.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  script/script_error.cpp \
  script/sign.cpp \
  script/standard.cpp \
  $(HODLCASH_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libhodlcash_util_a_CPPFLAGS = $(HODLCASH_INCLUDES)
libhodlcash_util_a_SOURCES = \
  support/pagelocker.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  random.cpp \
  rpcprotocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  uint256.cpp \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  $(HODLCASH_CORE_H)

if GLIBC_BACK_COMPAT
libhodlcash_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between HodlCash-cli and HodlCash-qt
libhodlcash_cli_a_CPPFLAGS = $(HODLCASH_INCLUDES)
libhodlcash_cli_a_SOURCES = \
  rpcclient.cpp \
  $(HODLCASH_CORE_H)

nodist_libhodlcash_util_a_SOURCES = $(srcdir)/obj/build.h
#

# HodlCashd binary #
HodlCashd_SOURCES = hodlcashd.cpp
HodlCashd_CPPFLAGS = $(HODLCASH_INCLUDES)
HodlCashd_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
HodlCashd_SOURCES += hodlcashd-res.rc
endif

HodlCashd_LDADD = \
  $(LIBHODLCASH_SERVER) \
  $(LIBHODLCASH_COMMON) \
  $(LIBHODLCASH_UNIVALUE) \
  $(LIBHODLCASH_UTIL) \
  $(LIBHODLCASH_CRYPTO) \
  $(LIBHODLCASH_CRYPTO_X86) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

if ENABLE_WALLET
HodlCashd_LDADD += libhodlcash_wallet.a
endif

HodlCashd_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS)
#

# HodlCash-cli binary #
HodlCash_cli_SOURCES = hodlcash-cli.cpp
HodlCash_cli_CPPFLAGS = $(HODLCASH_INCLUDES)
HodlCash_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
HodlCash_cli_SOURCES += hodlcash-cli-res.rc
endif

HodlCash_cli_LDADD = \
  $(LIBHODLCASH_CLI) \
  $(LIBHODLCASH_UTIL)

HodlCash_cli_LDADD += $(LIBLEVELDB) $(LIBMEMENV) $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS)
#

# HodlCash-tx binary #
HodlCash_tx_SOURCES = hodlcash-tx.cpp
HodlCash_tx_CPPFLAGS = $(HODLCASH_INCLUDES)
HodlCash_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
HodlCash_tx_SOURCES += hodlcash-tx-res.rc
endif

HodlCash_tx_LDADD = \
  $(LIBHODLCASH_UNIVALUE) \
  $(LIBHODLCASH_COMMON) \
  $(LIBHODLCASH_UTIL) \
  $(LIBHODLCASH_CRYPTO) \
  $(LIBHODLCASH_CRYPTO_X86) \
  $(LIBSECP256K1)

HodlCash_tx_LDADD += $(LIBLEVELDB) $(LIBMEMENV) $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# hodlcashconsensus library #
if BUILD_HODLCASH_LIBS
include_HEADERS = script/hodlcashconsensus.h
libhodlcashconsensus_la_SOURCES = \
  crypto/hmac_sha512.cpp \
  crypto/ripemd160.cpp \
  crypto/sha1.cpp \
  crypto/sha256.cpp \
  crypto/sha512.cpp \
  eccryptoverify.cpp \
  ecwrapper.cpp \
  hash.cpp \
  primitives/transaction.cpp \
  pubkey.cpp \
  script/hodlcashconsensus.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  uint256.cpp \
  utilstrencodings.cpp

if GLIBC_BACK_COMPAT
  libhodlcashconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libhodlcashconsensus_la_LDFLAGS = -no-undefined $(RELDFLAGS)
libhodlcashconsensus_la_LIBADD = $(CRYPTO_LIBS)
libhodlcashconsensus_la_CPPFLAGS = $(CRYPTO_CFLAGS) -I$(builddir)/obj -DBUILD_HODLCASH_INTERNAL

endif
#

CLEANFILES = leveldb/libleveldb.a leveldb/libmemenv.a
CLEANFILES += $(EXTRA_LIBRARIES)
CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno



DISTCLEANFILES = obj/build.h

EXTRA_DIST = leveldb

clean-local:
	-$(MAKE) -C leveldb clean
	-$(MAKE) -C secp256k1 clean
	rm -f leveldb/*/*.gcno leveldb/helpers/memenv/*.gcno
	-rm -f config.h

.rc.o:
	@test -f $(WINDRES)
	$(AM_V_GEN) $(WINDRES) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(CXXFLAGS)  -c -o $@ $<

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(abspath $(<D) $<)

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
