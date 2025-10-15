import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from pathlib import Path

# === CONFIG ===
INPUT_FILE = Path("outputs/photon_times.csv")
OUTPUT_PLOT = Path("analysis_out/s2_gaussian.png")
MIN_PHOTONS_PER_EVENT = 20      # per avere solo eventi "ricchi"
S1_CUTOFF_NS = 0.5              # rimuove eventuale S1 (solo fotoni dopo 0.5 ns)
NBINS = 6000               # istogramma temporale

# === LOAD DATA ===
df = pd.read_csv(INPUT_FILE, names=['event_id', 'pmt', 'time_ns'])
df = df[df['pmt'] == 'TopPMT'].copy()

if df.empty:
    raise ValueError("❌ Nessun dato del TopPMT trovato nel CSV.")

print(f"✅ Letti {len(df)} fotoni da {df['event_id'].nunique()} eventi.")

# === FILTRO S1 (se presente) ===
df = df[df['time_ns'] > S1_CUTOFF_NS]

# === ALLINEAMENTO PER EVENTO ===
# prendo il primo fotone per ogni evento come t0
t0 = df.groupby('event_id')['time_ns'].min()
df = df.join(t0, on='event_id', rsuffix='_t0')
df['t_aligned'] = df['time_ns'] - df['time_ns_t0']

# === EVENTI CON SUFFICIENTI FOTONI ===
counts = df.groupby('event_id').size()
good_events = counts[counts >= MIN_PHOTONS_PER_EVENT].index
df = df[df['event_id'].isin(good_events)]

if df.empty:
    raise ValueError("❌ Nessun evento con abbastanza fotoni dopo il filtraggio.")

# === COSTRUISCI ISTOGRAMMA GLOBALE ===
times = df['t_aligned'].to_numpy()
hist, bins = np.histogram(times, bins=NBINS)
bin_centers = 0.5 * (bins[:-1] + bins[1:])
bin_width = bins[1] - bins[0]

# ... (parte iniziale invariata: load, filtro, allineamento) ...

# === MODELLO ESPONENZIALE ===
def exp_decay(x, amp, tau, c0):
    return amp * np.exp(-x / tau) + c0

# Stime iniziali (solo x>0 per evitare fit su picco=0)
mask = bin_centers > 0
amp0 = hist.max()
tau0 = 2.0  # Dal tuo config scint_time
c00 = hist.min()
sigma_y = np.sqrt(np.maximum(hist, 1.0))

# === FIT ===
try:
    popt, pcov = curve_fit(
        exp_decay, bin_centers[mask], hist[mask],
        p0=[amp0, tau0, c00],
        sigma=sigma_y[mask], absolute_sigma=True,
        bounds=([0, 0.1, 0], [np.inf, 10, np.inf])
    )
    amp, tau, c0 = popt
except RuntimeError:
    raise RuntimeError("⚠️ Fit non convergente.")

# === PLOT ===
plt.figure(figsize=(7, 5))
plt.bar(bin_centers, hist, width=bin_width, align='center', alpha=0.6, label='data')
xfit = np.linspace(0, bin_centers.max(), 500)
plt.plot(xfit, exp_decay(xfit, *popt), 'r--', label=f'fit amp={amp:.2f}, τ={tau:.2f} ns')
plt.xlabel("t aligned [ns]")
plt.ylabel("counts / bin")
plt.legend()
plt.tight_layout()
plt.show()

print("---- FIT RESULT ----")
print(f"Amplitude: {amp:.2f}")
print(f"Tau (decay): {tau:.3f} ns")
print(f"Baseline:  {c0:.3f} counts/bin")
# === OUTPUT ===
print("---- FIT RESULT ----")
print(f"Amplitude: {amp:.2f}")
print(f"Mean:      {mean:.3f} ns")
print(f"Sigma:     {sigma:.3f} ns")
print(f"Baseline:  {c0:.3f} counts/bin")
OUTPUT_PLOT.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUTPUT_PLOT)
print(f"📈 Grafico salvato in: {OUTPUT_PLOT}")
