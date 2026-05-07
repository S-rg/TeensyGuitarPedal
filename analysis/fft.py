import numpy as np
import matplotlib.pyplot as plt
import librosa
import librosa.display

# Load audio
file_path = "/Users/suraj/Library/CloudStorage/OneDrive-PlakshaUniversity/Classes/Sem6/Embedded/Embedded-Project/analysis/testSampleRaw.mp3"
signal, sr = librosa.load(file_path, sr=None, mono=True)

# Compute STFT (Short-Time Fourier Transform)
n_fft = 2048        # window size
hop_length = 512    # step size

stft = librosa.stft(signal, n_fft=n_fft, hop_length=hop_length)

# Convert magnitude to decibels
spectrogram = librosa.amplitude_to_db(np.abs(stft), ref=np.max)

# Plot
plt.figure(figsize=(12, 6))
librosa.display.specshow(
    spectrogram,
    sr=sr,
    hop_length=hop_length,
    x_axis='time',
    y_axis='log'  # log scale is better for audio
)

plt.colorbar(label='Intensity (dB)')
plt.title("Spectrogram")
plt.tight_layout()
plt.show()
