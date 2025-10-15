import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import numpy as np
import glob
import os
import csv

def gaussian(x, amp, mean, sigma):
    return amp * np.exp(-(x - mean)**2 / (2 * sigma**2))

# Merge the lz_s2 per-thread files
output_dir = 'outputs/'
file_pattern = 'lz_s2_t*.csv'
merged_file = os.path.join(output_dir, 'lz_s2_merged.csv')

files = sorted(glob.glob(os.path.join(output_dir, file_pattern)))

if not files:
    print("No lz_s2 files found to merge.")
else:
    with open(merged_file, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        for i, filename in enumerate(files):
            with open(filename, 'r', newline='') as infile:
                reader = csv.reader(infile)
                if i == 0:
                    # No header in Geant4 CSV, so just write rows
                    writer.writerows(reader)
                else:
                    writer.writerows(reader)  # All files have no header
    print(f"Merged {len(files)} files into {merged_file}.")

# Load the merged lz_s2
df_lz = pd.read_csv(merged_file, names=['event_id', 'nPhotTop', 'Edep_GXe', 't_first_top_ns', 't_mean_top_ns'])

# Load photon_times
df_photon = pd.read_csv(os.path.join(output_dir, 'photon_times.csv'), names=['event_id', 'pmt', 'time_ns'])

# Merge to get t_mean per event
df = pd.merge(df_photon, df_lz[['event_id', 't_mean_top_ns']], on='event_id', how='left')

# Compute relative times
df['rel_time_ns'] = df['time_ns'] - df['t_mean_top_ns']

# Drop any rows where merge failed (NaN), though unlikely
df = df.dropna(subset=['rel_time_ns'])

# Histogram and fit
times = df['rel_time_ns'].values
hist, bins = np.histogram(times, bins=50)
bin_centers = (bins[:-1] + bins[1:]) / 2
popt, _ = curve_fit(gaussian, bin_centers, hist, p0=[max(hist), 0, 1])  # Initial guess centered at 0
plt.figure()
plt.hist(times, bins=50, label='Data')
plt.plot(bin_centers, gaussian(bin_centers, *popt), 'r--', label='Gaussian fit')
plt.title('S2 Signal Time Distribution (Relative to Mean)')
plt.xlabel('Relative Time (ns)')
plt.ylabel('Counts')
plt.legend()
plt.show()
os.makedirs('analysis_out', exist_ok=True)
plt.savefig('analysis_out/s2_gaussian.png')

# Additional plots for everything
# Histogram of nPhotTop
plt.figure()
plt.hist(df_lz['nPhotTop'], bins=50)
plt.title('Number of Photons per Event (Top PMT)')
plt.xlabel('nPhotTop')
plt.ylabel('Events')
plt.show()
plt.savefig('analysis_out/nPhotTop_hist.png')

# Histogram of Edep_GXe
plt.figure()
plt.hist(df_lz['Edep_GXe'], bins=50)
plt.title('Energy Deposited in GXe per Event')
plt.xlabel('Edep (MeV)')
plt.ylabel('Events')
plt.show()
plt.savefig('analysis_out/Edep_hist.png')

# Histogram of t_mean_top_ns
plt.figure()
plt.hist(df_lz['t_mean_top_ns'], bins=50)
plt.title('Mean Photon Time per Event')
plt.xlabel('t_mean_top_ns (ns)')
plt.ylabel('Events')
plt.show()
plt.savefig('analysis_out/t_mean_hist.png')

# Histogram of t_first_top_ns
plt.figure()
plt.hist(df_lz['t_first_top_ns'], bins=50)
plt.title('First Photon Time per Event')
plt.xlabel('t_first_top_ns (ns)')
plt.ylabel('Events')
plt.show()
plt.savefig('analysis_out/t_first_hist.png')