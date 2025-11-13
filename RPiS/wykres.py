import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# --- 1. Dane wejściowe ---
X3 = np.array([0.933333333, 0.929487179, 0.919230769, 0.919230769, 0.915384615, 0.91025641, 0.907692308, 0.903846154, 0.896153846, 0.88974359, 0.880769231,
               0.871794872, 0.862820513, 0.85, 0.825641026, 0.787179487, 0.752564103, 0.703846154, 0.638461538, 0.525641026, 0.435897436, 0.366666667, 0.176923077])

Y3 = np.array([0.009375, 0.01775, 0.035125, 0.04675, 0.059375, 0.071875, 0.085625, 0.09875, 0.112625, 0.124625, 0.13875,
               0.149375, 0.163875, 0.1777125, 0.1925, 0.215125, 0.229375, 0.247625, 0.26675, 0.29275, 0.308625, 0.319375, 0.346125])

X2 = np.array([0.000286, 0.025571, 0.053143, 0.088143, 0.118857, 0.138857, 0.176571, 0.203857, 0.244643, 0.282857, 0.324571,
              0.367429, 0.392571, 0.424786, 0.452357, 0.483, 0.513071, 0.538214, 0.565143, 0.576786, 0.585929, 0.6015, 0.6175])

Y2 = np.array([0.223636364, 0.221818182, 0.221818182, 0.22, 0.22, 0.218181818, 0.218181818, 0.216363636, 0.216363636, 0.214545455, 0.212727273,
              0.209090909, 0.207272727, 0.203636364, 0.201818182, 0.194545455, 0.185454545, 0.174545455, 0.152727273, 0.141818182, 0.130909091, 0.107272727, 0.08])

X1 = np.array([1.511111111, 1.43015873, 1.319047619, 1.219047619, 1.103174603, 0.99047619, 0.906349206, 0.793650794, 0.695238095, 0.634920635, 0.579365079,
              0.523809524, 0.480952381, 0.43015873, 0.349206349, 0.315873016, 0.274603175, 0.238095238, 0.184126984, 0.152380952, 0.10952381, 0.09047619, 0.074603175])

Y1 = np.array([0.301, 0.348, 0.379, 0.396, 0.41, 0.421, 0.427, 0.435, 0.441, 0.443, 0.446,
              0.448, 0.45, 0.452, 0.455, 0.456, 0.457, 0.459, 0.459, 0.461, 0.463, 0.462, 0.463])
# --- 2. Definicja funkcji modelu ---


def model_exponencjalny(x, a, b, c):
    return a * np.exp(b * x) + c

# --- 3. Dopasowanie modelu (Scipy) ---


# Wartości startowe (p0). Te z `polyfit` są całkiem dobre.
wartosci_startowe = [-5, 12, -2]

# === POPRAWKA 1: Zwiększamy maksymalną liczbę iteracji ===
# Dodajemy argument maxfev (domyślnie 800)
try:
    Y1 = -Y1
    Y2 = -Y2
    Y3 = -Y3
    plt.figure(figsize=(10, 6))
    # plt.scatter(X3, Y3, label='Oryginalne dane', color='blue')
    popt2, _ = curve_fit(model_exponencjalny, X1, Y1,
                         p0=wartosci_startowe,
                         maxfev=10000)  # Zwiększono limit do 10 000
    a1, b1, c1 = popt2
    X_fit1 = np.linspace(X1.min(), X1.max(), 100)
    Y_fit1 = model_exponencjalny(X_fit1, a1, b1, c1)
    plt.plot(X_fit1, Y_fit1, color='red', label="Ogniwo monokrystaliczne")

    popt2, _ = curve_fit(model_exponencjalny, X2, Y2,
                         p0=wartosci_startowe,
                         maxfev=10000)  # Zwiększono limit do 10 000
    a2, b2, c2 = popt2
    X_fit2 = np.linspace(X2.min(), X2.max(), 100)
    Y_fit2 = model_exponencjalny(X_fit2, a2, b2, c2)
    plt.plot(X_fit2, Y_fit2, color='green', label="Ogniwo amorficzne")

    popt3, _ = curve_fit(model_exponencjalny, X3, Y3,
                         p0=wartosci_startowe,
                         maxfev=10000)  # Zwiększono limit do 10 000
    a3, b3, c3 = popt3

    X_fit3 = np.linspace(X3.min(), X3.max(), 100)
    Y_fit3 = model_exponencjalny(X_fit3, a3, b3, c3)
    plt.plot(X_fit3, Y_fit3, color='blue', label="Ogniwo polikrystaliczne")

    plt.xlabel('U/n [V]')
    plt.ylabel('j [mA/cm^2]')
    plt.legend()
    plt.grid(True)
    plt.show()

except RuntimeError as e:
    print(f"Dopasowanie nie powiodło się: {e}")
    print("Spróbuj dostosować wartości startowe (p0) lub zwiększyć maxfev.")
