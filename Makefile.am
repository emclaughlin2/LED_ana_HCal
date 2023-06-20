AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -I$(OPT_SPHENIX)/include  \
  -I/home/repo/Debian/root/include \  
  -I`root-config --incdir`

#-I$(ROOTSYS)/include \
#AM_LDFLAGS = \
#  -L$(libdir) \
#  -L$(OFFLINE_MAIN)/lib \
#  -L$(OFFLINE_MAIN)/lib64

pkginclude_HEADERS = \
  saHCal.h

lib_LTLIBRARIES = \
  libsaHCal.la

libsaHCal_la_SOURCES = \
  saHCal.cc

libsaHCal_la_LIBADD = \
  -L$(OPT_SPHENIX)/lib \
  -lodbc++ \
  -lcalo_io \
  -lcalo_reco
