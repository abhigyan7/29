##
# 29 bot
#
# @file
# @version 0.1

CXX=g++
RM=rm -f
CXXFLAGS=-g -std=c++20

SRCS=game.cc bot.cc ismcts_tn.cc
OBJS=$(subst .cc,.o,$(SRCS))

all: bot

bot: $(OBJS)
	$(CXX) -o bot $(OBJS) $(CXXFLAGS)

optim:
	$(CXX) -o bot $(SRCS) -std=c++20 -Ofast -static -flto

clean:
	$(RM) $(OBJS) bot

# end
