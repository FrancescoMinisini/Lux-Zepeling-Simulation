# ===========================
# Makefile - LZ S2 Simulation
# ===========================

BUILD_DIR      ?= build
GEANT4_DIR     ?= $(HOME)/geant4-install/lib/Geant4-11.2.2
CMAKE          ?= cmake
NPROC          := $(shell nproc)
JOBS           ?= $(NPROC)

EXEC           := $(BUILD_DIR)/LZSim
MACROS_DIR     := macros
OUTPUTS_DIR    := outputs
ANALYSIS_OUT   := analysis_out
ANALYZE_SCRIPT := analyze_s2_datasets.py  # Assume you adapt your Python script

S2_ROOT        := $(OUTPUTS_DIR)/lz_s2.root

.PHONY: help build all $(EXEC) runs s2 analyze clean distclean check

help:
	@echo ""
	@echo "Targets:"
	@echo "  make build        -> configure and compile"
	@echo "  make all          -> alias for build"
	@echo "  make runs         -> run s2"
	@echo "  make s2           -> run macros/run_s2.mac (output in outputs/)"
	@echo "  make analyze      -> run $(ANALYZE_SCRIPT) on outputs/"
	@echo "  make clean        -> rm $(BUILD_DIR)"
	@echo "  make distclean    -> clean + rm $(OUTPUTS_DIR) $(ANALYSIS_OUT)"
	@echo "  make check        -> ls $(OUTPUTS_DIR)"
	@echo ""

build: $(EXEC)

all: build

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(EXEC): | $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) -DGeant4_DIR=$(GEANT4_DIR) ../
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)

$(OUTPUTS_DIR):
	@mkdir -p $(OUTPUTS_DIR)

runs: s2

s2: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running S2 simulation"
	@cd $(BUILD_DIR) && ./LZSim ../$(MACROS_DIR)/run_s2.mac > ../LOG.txt
	@echo "==> Output: $(S2_ROOT)"
vis: vis_s2

vis_s2: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running S2 simulation"
	@cd $(BUILD_DIR) && ./LZSim ../$(MACROS_DIR)/vis_s2.mac > ../LOG.txt
	@echo "==> Output: $(S2_ROOT)"

analyze:
	@echo "==> Analyzing for gaussian shape"
	@python3 $(ANALYZE_SCRIPT) --input $(OUTPUTS_DIR) --out $(ANALYSIS_OUT)

check:
	@ls -lh $(OUTPUTS_DIR) || echo "(empty)"

clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@rm -rf $(OUTPUTS_DIR) $(ANALYSIS_OUT)