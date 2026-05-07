import numpy as np
import matplotlib

matplotlib.use("Qt5Agg")

import matplotlib.pyplot as plt

# ===== USER INPUT =====
sample_rate = 44100

freqs = np.array([31.25, 62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000])

gains_db = np.array([0, 0, -4, 0, 0, 0, 0, 0, 2, 0])

Qs = np.ones_like(freqs) * 1.2


# ===== BIQUAD DESIGN =====
def peaking_eq(fs, f0, Q, gain_db):
    A = 10 ** (gain_db / 40)
    w0 = 2 * np.pi * f0 / fs
    alpha = np.sin(w0) / (2 * Q)

    b0 = 1 + alpha * A
    b1 = -2 * np.cos(w0)
    b2 = 1 - alpha * A

    a0 = 1 + alpha / A
    a1 = -2 * np.cos(w0)
    a2 = 1 - alpha / A

    # normalize
    b = np.array([b0, b1, b2]) / a0
    a = np.array([1.0, a1 / a0, a2 / a0])

    return b, a


# ===== FREQUENCY RESPONSE =====
def freq_response(b, a, w):
    # H(e^jw)
    ejw = np.exp(-1j * w)
    ejw2 = np.exp(-2j * w)

    num = b[0] + b[1] * ejw + b[2] * ejw2
    den = a[0] + a[1] * ejw + a[2] * ejw2

    return num / den


# ===== BUILD CASCADE =====
def compute_total_response(fs, freqs, gains_db, Qs):
    w = np.linspace(0, np.pi, 4096)
    H_total = np.ones_like(w, dtype=complex)

    for f0, g, Q in zip(freqs, gains_db, Qs):
        b, a = peaking_eq(fs, f0, Q, g)
        H = freq_response(b, a, w)
        H_total *= H  # cascade

    return w, H_total


# ===== RUN =====
w, H = compute_total_response(sample_rate, freqs, gains_db, Qs)

# Convert to Hz
freq_axis = w * sample_rate / (2 * np.pi)

# Magnitude in dB
magnitude_db = 20 * np.log10(np.abs(H) + 1e-12)

# ===== PLOT =====
plt.figure()
plt.semilogx(freq_axis, magnitude_db)
plt.title("10-Band EQ Frequency Response")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude (dB)")
plt.grid(True, which="both")

plt.scatter(freqs, gains_db)

plt.show()
