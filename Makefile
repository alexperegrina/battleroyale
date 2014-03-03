
# Config

OPTIMIZE = 2 # Optimization level (0 to 3)
DEBUG    = 0 # Compile for debugging (0 or 1)
PROFILE  = 0 # Compile for profile (0 or 1)

# Do not edit past this line

# The following two lines will detect all your players (files matching "AI*.cc")

PLAYERS_SRC = $(wildcard AI*.cc)
PLAYERS_OBJ = $(patsubst %.cc, %.o, $(PLAYERS_SRC)) AITonto.o

# Flags

ifeq ($(strip $(PROFILE)),1)
	PROFILEFLAGS=-pg
endif
ifeq ($(strip $(DEBUG)),1)
	DEBUGFLAGS=-DDEBUG -g -rdynamic
endif

CXXFLAGS = -Wall -Wno-unused-variable $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIMIZE))
LDFLAGS  = -lm $(PROFILEFLAGS) $(DEBUGFLAGS) -O$(strip $(OPTIM))

# Rules

all: BattleRoyale 

clean:
	rm -rf BattleRoyale SecBattleRoyale *.o *.exe Makefile.deps refman.pdf
	rm -rf latex

BattleRoyale: BackTrace.o Utils.o PosDir.o Board.o Action.o Player.o Registry.o Game.o BattleRoyale.o $(PLAYERS_OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

SecBattleRoyale: BackTrace.o Utils.o PosDir.o Board.o Action.o Player.o Registry.o SecGame.o SecBattleRoyale.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.exe: %.o BackTrace.o Utils.o PosDir.o Board.o Action.o Player.o Registry.o SecGame.o SecBattleRoyale.o
	$(CXX) $(LDFLAGS)  $^ -o $@

#%.o: %.cc
#	$(CXX) $(CXXFLAGS) $(CFLAGS) $< -c -o $@

obj:
	mkdir obj

Makefile.deps: *.cc
	$(CXX) $(CPPFLAGS) -MM *.cc > Makefile.deps

doxygen:
	doxygen
	cd latex && make
	cp latex/refman.pdf .

include Makefile.deps


AITonto.o: AITonto.o-linux64
	cp AITonto.o-linux64 AITonto.o
