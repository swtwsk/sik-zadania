SHELL = /bin/sh

#MAKRA
CXX := g++
CXXFLAGS += -Wall -O2 -std=c++17 -pthread
LDFLAGS += -pthread
INCLUDEFLAGS := -Wall -O2 -pthread

#KOMPILACJA
.PHONY: all clean

#KOMPILACJA NADAJNIKA
SENDER_DIR := sender
SENDER_OBJDIR := sendobj
SENDER_SRCS := $(wildcard $(SENDER_DIR)/*.cc)
SENDER_OBJS := $(subst $(SENDER_DIR)/,$(SENDER_OBJDIR)/,$(SENDER_SRCS:.cc=.o))

all: sikradio-sender

sikradio-sender: $(SENDER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
$(SENDER_OBJDIR)/%.o: $(SENDER_DIR)/%.cc
	@mkdir -p $(SENDER_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#CZYSZCZENIE
clean: sikradio-sender
	@rm -rf $(SENDER_OBJDIR)
	@rm -f sikradio-sender

