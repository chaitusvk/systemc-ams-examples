# ============================================================
#  Makefile  –  Single-Ended SerDes  |  SystemC-AMS
#
#  Requires:
#    SYSTEMC_HOME   = path to SystemC 2.3.x installation
#    SYSTEMCAMS_HOME= path to SystemC-AMS 2.x installation
# ============================================================

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

# ── Paths (override via environment) ─────────────────────────
SYSTEMC_HOME    ?= /usr/local/systemc
SYSTEMCAMS_HOME ?= /usr/local/systemc-ams

INCS := -I$(SYSTEMC_HOME)/include \
        -I$(SYSTEMCAMS_HOME)/include \
        -Isrc

LIBS := -L$(SYSTEMC_HOME)/lib-linux64    -lsystemc \
        -L$(SYSTEMCAMS_HOME)/lib-linux64 -lsystemc-ams \
        -lm

SRC  := tb/serdes_tb.cpp

TARGET := sim/serdes_sim

.PHONY: all clean run waves

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p sim
	$(CXX) $(CXXFLAGS) $(INCS) $< -o $@ $(LIBS)
	@echo "Build successful → $(TARGET)"

run: all
	@cd sim && ../$(TARGET)

waves:
	@gtkwave sim/serdes_tb.vcd &

clean:
	rm -f $(TARGET) sim/*.dat sim/*.vcd
