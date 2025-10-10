import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import numpy as np

def gaussian(x, amp, mean, sigma):
    return amp * np.exp(-(x - mean)**2 / (2 * sigma**2))

df = pd.read_csv('outputs/photon_times.csv', names=['event_id', 'pmt', 'time_ns'])
times = df['time_ns'].values  # Or group by event
hist, bins = np.histogram(times, bins=50)
bin_centers = (bins[:-1] + bins[1:]) / 2
popt, _ = curve_fit(gaussian, bin_centers, hist)
plt.hist(times, bins=50)
plt.plot(bin_centers, gaussian(bin_centers, *popt), 'r--')
plt.savefig('analysis_out/s2_gaussian.png')