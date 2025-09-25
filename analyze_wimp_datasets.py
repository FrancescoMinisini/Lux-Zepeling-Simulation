#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import os
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")  # no GUI backend (WSL/CI-safe)
import matplotlib.pyplot as plt

try:
    import uproot
except ImportError as e:
    raise SystemExit(
        "Missing dependency 'uproot'. Install with:\n"
        "  pip install uproot awkward numpy matplotlib pandas"
    ) from e


CATEGORIES = {
    "wimp_single.root":       "single",
    "wimp_double_near.root":  "double_near",
    "wimp_double_far.root":   "double_far",
    "wimp_triple.root":       "triple",
}

COLUMNS = [
    ("event_id",          "i4"),
    ("nPhotTop",          "i4"),
    ("nPhotBot",          "i4"),
    ("Edep_LXe",          "f8"),  # MeV
    ("t_first_top_ns",    "f8"),
    ("t_first_bot_ns",    "f8"),
    ("t_mean_top_ns",     "f8"),
    ("t_mean_bot_ns",     "f8"),
]


def load_tree_to_df(root_path: Path, preferred_name: str) -> pd.DataFrame:
    with uproot.open(root_path) as f:
        # Collect keys and keep only TTrees
        keys = []
        for k in f.keys():
            # Remove cycles and leading slashes
            base = k.split(";")[0].lstrip("/")
            try:
                obj = f[base]
                from uproot.behaviors.TTree import TTree
                if isinstance(obj, TTree):
                    keys.append(base)
            except Exception:
                pass

        if not keys:
            raise RuntimeError(f"Nessun TTree in {root_path.name}. Chiavi: {list(f.keys())}")

        tree_name = preferred_name if preferred_name in keys else keys[0]
        if preferred_name not in keys:
            print(f"[INFO] Using tree '{tree_name}' in {root_path.name}")

        t = f[tree_name]
        cols_present = [c for c, _ in COLUMNS if c in t.keys()]
        arrs = t.arrays(cols_present, library="np")
        df = pd.DataFrame(arrs)
        for c, _ in COLUMNS:
            if c not in df:
                df[c] = np.nan
        return df


def quick_stats(df: pd.DataFrame) -> dict:
    out = {}
    out["n"] = int(len(df))
    out["nPhotTop_mean"] = float(df["nPhotTop"].mean())
    out["nPhotBot_mean"] = float(df["nPhotBot"].mean())
    out["Edep_mean_MeV"] = float(df["Edep_LXe"].mean())
    out["t_first_top_ns_mean"] = float(df["t_first_top_ns"].mean())
    out["t_first_bot_ns_mean"] = float(df["t_first_bot_ns"].mean())
    return out


def plot_hist(ax, data, title, xlabel, bins=60):
    data = np.asarray(data)
    data = data[~np.isnan(data)]
    if len(data) == 0:
        ax.text(0.5, 0.5, "no data", ha="center", va="center")
    else:
        ax.hist(data, bins=bins)
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel("counts")


def plot_scatter(ax, x, y, title, xlabel, ylabel, s=5, alpha=0.4):
    x = np.asarray(x)
    y = np.asarray(y)
    m = (~np.isnan(x)) & (~np.isnan(y))
    if m.sum() == 0:
        ax.text(0.5, 0.5, "no data", ha="center", va="center")
    else:
        ax.scatter(x[m], y[m], s=s, alpha=alpha)
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)


def analyze_category(root_dir: Path, out_dir: Path, file_name: str, tree_hint: str):
    root_path = root_dir / file_name
    if not root_path.exists():
        print(f"[WARN] Missing file: {root_path}")
        return None

    print(f"[INFO] Reading {root_path} :: tree hint '{tree_hint}'")
    df = load_tree_to_df(root_path, tree_hint)

    # Save CSV for ML
    csv_path = out_dir / f"{tree_hint}.csv"
    df.to_csv(csv_path, index=False)
    print(f"[OK] CSV saved -> {csv_path}")

    # Plots
    figdir = out_dir / "figs"
    figdir.mkdir(parents=True, exist_ok=True)

    # 1) nPhotTop & nPhotBot hist
    fig, ax = plt.subplots(figsize=(6,4))
    plot_hist(ax, df["nPhotTop"].values, f"{tree_hint}: nPhotTop", "nPhotTop")
    fig.tight_layout()
    fig.savefig(figdir / f"{tree_hint}_nPhotTop.png", dpi=150)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(6,4))
    plot_hist(ax, df["nPhotBot"].values, f"{tree_hint}: nPhotBot", "nPhotBot")
    fig.tight_layout()
    fig.savefig(figdir / f"{tree_hint}_nPhotBot.png", dpi=150)
    plt.close(fig)

    # 2) Edep histogram
    fig, ax = plt.subplots(figsize=(6,4))
    plot_hist(ax, df["Edep_LXe"].values, f"{tree_hint}: Edep in LXe", "Edep [MeV]")
    fig.tight_layout()
    fig.savefig(figdir / f"{tree_hint}_Edep.png", dpi=150)
    plt.close(fig)

    # 3) First-hit timing correlation (top vs bot)
    fig, ax = plt.subplots(figsize=(5,5))
    plot_scatter(ax,
                 df["t_first_top_ns"].values,
                 df["t_first_bot_ns"].values,
                 f"{tree_hint}: t_first_top vs t_first_bot",
                 "t_first_top [ns]", "t_first_bot [ns]")
    fig.tight_layout()
    fig.savefig(figdir / f"{tree_hint}_tfirst_corr.png", dpi=150)
    plt.close(fig)

    # 4) Total photons vs Edep
    tot = (df["nPhotTop"].values + df["nPhotBot"].values).astype(float)
    fig, ax = plt.subplots(figsize=(5,5))
    plot_scatter(ax,
                 df["Edep_LXe"].values, tot,
                 f"{tree_hint}: Total PE vs Edep",
                 "Edep [MeV]", "nPhotTop + nPhotBot")
    fig.tight_layout()
    fig.savefig(figdir / f"{tree_hint}_PE_vs_Edep.png", dpi=150)
    plt.close(fig)

    stats = quick_stats(df)
    return stats


def main():
    ap = argparse.ArgumentParser(description="Analyze WIMP datasets from Geant4 ROOT files")
    ap.add_argument("--input",  type=str, default="../outputs",
                    help="Directory containing ROOT files (default: ../outputs)")
    ap.add_argument("--out",    type=str, default="./analysis_out",
                    help="Directory to write CSV and figures (default: ./analysis_out)")
    args = ap.parse_args()

    root_dir = Path(args.input).resolve()
    out_dir  = Path(args.out).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"[INFO] Input dir : {root_dir}")
    print(f"[INFO] Output dir: {out_dir}")

    summary = {}
    for file_name, tree_hint in CATEGORIES.items():
        stats = analyze_category(root_dir, out_dir, file_name, tree_hint)
        if stats:
            summary[tree_hint] = stats

    # Save quick summary
    if summary:
        df_sum = pd.DataFrame(summary).T
        sum_path = out_dir / "summary.csv"
        df_sum.to_csv(sum_path)
        print(f"[OK] Summary saved -> {sum_path}")
        print(df_sum)

        files_present = [k for k in CATEGORIES.values() if (out_dir / "figs" / f"{k}_nPhotTop.png").exists()]
        if files_present:
            print("[INFO] Generated per-category figures in:", out_dir / "figs")
    else:
        print("[WARN] No categories processed. Check input path and file names.")


if __name__ == "__main__":
    main()
