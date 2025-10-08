import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit

# Funzione esponenziale per il fit
def exp_decay(t, A, tau):
    return A * np.exp(-t / tau)

# Carica i tempi dei fotoni
data = pd.read_csv('outputs/photon_times.csv', names=['event_id', 'pmt', 'time_ns'])

# Separa i tempi per PMT
times_top = data[data['pmt'] == 'TopPMT']['time_ns']
times_bot = data[data['pmt'] == 'BottomPMT']['time_ns']

# Crea istogramma
plt.figure(figsize=(10, 6))
hist_top, bins_top, _ = plt.hist(times_top, bins=50, density=True, alpha=0.5, label='Top PMT')
hist_bot, bins_bot, _ = plt.hist(times_bot, bins=50, density=True, alpha=0.5, label='Bottom PMT')
plt.xlabel('Time (ns)')
plt.ylabel('Density')
plt.title('Signal Shape')

# Fit esponenziale
bin_centers_top = (bins_top[:-1] + bins_top[1:]) / 2
bin_centers_bot = (bins_bot[:-1] + bins_bot[1:]) / 2

# Fit per Top PMT
if len(times_top) > 1:
    popt_top, _ = curve_fit(exp_decay, bin_centers_top, hist_top, p0=[1.0, 2.2])
    plt.plot(bin_centers_top, exp_decay(bin_centers_top, *popt_top), 'b--', 
             label=f'Top PMT Fit (τ={popt_top[1]:.2f} ns)')

# Fit per Bottom PMT
if len(times_bot) > 1:
    popt_bot, _ = curve_fit(exp_decay, bin_centers_bot, hist_bot, p0=[1.0, 2.2])
    plt.plot(bin_centers_bot, exp_decay(bin_centers_bot, *popt_bot), 'r--', 
             label=f'Bottom PMT Fit (τ={popt_bot[1]:.2f} ns)')
tau = 2.2  # Costante temporale (ns)
x = np.linspace(0, max(max(times_top, default=10), max(times_bot, default=10)), 100)
y = (1/tau) * np.exp(-x/tau)
plt.plot(x, y, 'r--', label=f'Exp decay (τ={tau} ns)')
plt.legend()
plt.savefig('outputs/signal_shape.png')
plt.show()

print(f"Top PMT: Fitted τ = {popt_top[1]:.2f} ns")
print(f"Bottom PMT: Fitted τ = {popt_bot[1]:.2f} ns")