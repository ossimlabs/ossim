
include ../../../Makefile.common
include ../../../makelib.mak
LIBRARY   =
INCLUDES  = $(BASE_INCLUDES)
SRCS=$(wildcard *.cc)
OBJECTS=$(patsubst %.cc,%.o,$(SRCS)) 

.cc:
	$(CXX) $(LIBRARY) $<  $(LIBRARY) -o $@

.cc.o:
	$(CXX) $(INCLUDES) $< -c

all: $(OBJECTS) image_open$(EXEEXT) image_copy$(EXEEXT) \
	image_copy_b1$(EXEEXT) reproject$(EXEEXT) kwl$(EXEEXT) \
	image_data$(EXEEXT) band_average$(EXEEXT)
#\
#	shared_band_average$(EXEEXT)

image_open$(EXEEXT): image_open.o
	$(CXX) $(INCLUDES) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) image_open.o $(OSSIM_APP_LINK) -o $@

image_copy$(EXEEXT): image_copy.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) image_copy.o $(OSSIM_APP_LINK) -o $@

image_copy_b1$(EXEEXT): image_copy_b1.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) image_copy_b1.o $(OSSIM_APP_LINK) -o $@

reproject$(EXEEXT): reproject.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) reproject.o $(OSSIM_APP_LINK) -o $@

kwl$(EXEEXT): kwl.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) kwl.o $(OSSIM_APP_LINK) -o $@

image_data$(EXEEXT): image_data.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) image_data.o $(OSSIM_APP_LINK) -o $@

band_average$(EXEEXT): band_average.o
	$(CXX) $(CXXFLAGS) $(CDEBUGFLAGS) $(BASE_DEFINES) $(LDFLAGS_APP) band_average.o $(OSSIM_APP_LINK) -o $@
