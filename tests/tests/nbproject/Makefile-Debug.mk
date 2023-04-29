#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc-11
CCC=g++-11
CXX=g++-11
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/29dd86f/virtualfpu.o \
	${OBJECTDIR}/tests.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=--std=c++2a
CXXFLAGS=--std=c++2a

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath,'../..'

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/29dd86f/virtualfpu.o: ../../virtualfpu.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/29dd86f
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/29dd86f/virtualfpu.o ../../virtualfpu.cpp

${OBJECTDIR}/tests.o: tests.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tests.o tests.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
