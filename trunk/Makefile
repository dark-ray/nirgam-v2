# makefile

# architecture
TARGET_ARCH = linux

# compiler
CC     = g++ 

# debug flag
DEBUG  = -g
OTHER  = -Wall
LDFLAG = -rdynamic

#CFLAGS = $(DEBUG) $(OTHER) $(LDFLAG)
CFLAGS = $(DEBUG) $(LDFLAG)

MODULE = nirgam

CORE_SRCS = \
	core/rng.cpp \
	core/fifo.cpp \
	core/router.cpp \
	core/rr_arbiter.cpp \
	core/RouteInfo.cpp \
	core/InputChannel.cpp \
	core/ipcore.cpp \
	core/OutputChannel.cpp \
	core/VCAllocator.cpp \
	core/NWTile.cpp \
	core/NoC.cpp \
	core/main.cpp \
	core/Controller.cpp \
	core/ranvar.cpp \
	application/src/TG.cpp

APP_SRCS = \
	application/src/App_send.cpp \
	application/src/App_concat.cpp \
	application/src/App_recv.cpp \
	application/src/CBR.cpp \
	application/src/Bursty.cpp \
	application/src/Trace_traffic.cpp \
	application/src/Sink.cpp

ROUTER_SRCS = \
	router/src/OE_router.cpp \
	router/src/XY_router.cpp \
	router/src/source_router.cpp \
	router/src/DyAD_OE_router.cpp \
	router/src/West_First_router.cpp \
	router/src/North_Last_router.cpp \
	router/src/Negative_First_router.cpp \
	router/src/DyBM_router.cpp \
	router/src/DyXY_FT_router.cpp \
	router/src/DyXY_router.cpp	

CORE_OBJS = $(CORE_SRCS:.cpp=.o)

APP_OBJS = $(APP_SRCS:.cpp=.o)

ROUTER_OBJS = $(ROUTER_SRCS:.cpp=.o)

APP_LIB = $(subst src,lib,$(APP_OBJS:.o=.so))

ROUTER_LIB = $(subst src,lib,$(ROUTER_OBJS:.o=.so))

include Makefile.defs
# DO NOT DELETE
