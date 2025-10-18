# ===========================
# Makefile - WIMP Simulation
# ===========================

# ---- Configurazione variabili (sovrascrivibili da CLI) ----
BUILD_DIR      ?= build
GEANT4_DIR     ?= $(HOME)/geant4-install/lib/Geant4-11.2.2
CMAKE          ?= cmake
NPROC          := $(shell nproc)
JOBS           ?= $(NPROC)

EXEC           := $(BUILD_DIR)/Test
MACROS_DIR     := macros
OUTPUTS_DIR    := outputs
ANALYSIS_OUT   := analysis_out
ANALYZE_SCRIPT := analyze_wimp_datasets.py

# Filtri/nomi file ROOT prodotti (le macro devono già impostare /analysis/setFileName ../outputs/...)
SINGLE_ROOT       := $(OUTPUTS_DIR)/wimp_single.root
DOUBLE_NEAR_ROOT  := $(OUTPUTS_DIR)/wimp_double_near.root
DOUBLE_FAR_ROOT   := $(OUTPUTS_DIR)/wimp_double_far.root
TRIPLE_ROOT       := $(OUTPUTS_DIR)/wimp_triple.root

.PHONY: help build all $(EXEC) runs single double_near double_far triple analyze clean distclean check

# ---- Help ----
help:
	@echo ""
	@echo "Targets principali:"
	@echo "  make build        -> configura e compila (CMake in $(BUILD_DIR), Geant4 in $(GEANT4_DIR))"
	@echo "  make all          -> alias di 'build'"
	@echo "  make runs         -> esegue tutte le macro (single, double_near, double_far, triple)"
	@echo "  make single       -> esegue macros/run_single.mac (scrive ROOT in ../outputs)"
	@echo "  make double_near  -> esegue macros/run_double_near.mac"
	@echo "  make double_far   -> esegue macros/run_double_far.mac"
	@echo "  make triple       -> esegue macros/run_triple.mac"
	@echo "  make analyze      -> lancia $(ANALYZE_SCRIPT) su outputs/ e salva in $(ANALYSIS_OUT)/"
	@echo "  make clean        -> rimuove la cartella $(BUILD_DIR)"
	@echo "  make distclean    -> clean + rimuove $(OUTPUTS_DIR) e $(ANALYSIS_OUT)"
	@echo "  make check        -> lista i file in $(OUTPUTS_DIR)"
	@echo ""
	@echo "Variabili sovrascrivibili:"
	@echo "  GEANT4_DIR=$(GEANT4_DIR)"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  JOBS=$(JOBS)"
	@echo ""

# ---- Build: configura + compila ----
build: $(EXEC)

all: build

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(EXEC): | $(BUILD_DIR)
	@echo "==> Configuring with CMake (Geant4_DIR=$(GEANT4_DIR))"
	@cd $(BUILD_DIR) && $(CMAKE) -DGeant4_DIR=$(GEANT4_DIR) ../
	@echo "==> Building with make -j$(JOBS)"
	@$(MAKE) -C $(BUILD_DIR) -j$(JOBS)

# ---- Utility: outputs dir fuori da build ----
$(OUTPUTS_DIR):
	@mkdir -p $(OUTPUTS_DIR)

# ---- Run singoli (si parte dalla root e si esegue il binario dentro build) ----
# Le macro usano "/control/execute macros/common_batch.mac" e sono referenziate
# dalla build con path relativo "../macros/..." così gli output vanno in ../outputs.

runs: single double_near double_far triple

single: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running SINGLE"
	@cd $(BUILD_DIR) && ./Test ../$(MACROS_DIR)/run_single.mac
	@echo "==> Output atteso: $(SINGLE_ROOT)"

double_near: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running DOUBLE NEAR"
	@cd $(BUILD_DIR) && ./Test ../$(MACROS_DIR)/run_double_near.mac
	@echo "==> Output atteso: $(DOUBLE_NEAR_ROOT)"

double_far: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running DOUBLE FAR"
	@cd $(BUILD_DIR) && ./Test ../$(MACROS_DIR)/run_double_far.mac
	@echo "==> Output atteso: $(DOUBLE_FAR_ROOT)"

triple: $(EXEC) | $(OUTPUTS_DIR)
	@echo "==> Running TRIPLE"
	@cd $(BUILD_DIR) && ./Test ../$(MACROS_DIR)/run_triple.mac
	@echo "==> Output atteso: $(TRIPLE_ROOT)"

# ---- Analisi con Python (uproot) ----
analyze:
	@echo "==> Running analysis: $(ANALYZE_SCRIPT) --input $(OUTPUTS_DIR) --out $(ANALYSIS_OUT)"
	@python3 $(ANALYZE_SCRIPT) --input $(OUTPUTS_DIR) --out $(ANALYSIS_OUT)

# ---- Utility ----
check:
	@echo "==> Contenuto di $(OUTPUTS_DIR):"
	@ls -lh $(OUTPUTS_DIR) 2>/dev/null || echo "(cartella vuota o inesistente)"

clean:
	@echo "==> Removing $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo "==> Removing $(OUTPUTS_DIR) and $(ANALYSIS_OUT)"
	@rm -rf $(OUTPUTS_DIR) $(ANALYSIS_OUT)
