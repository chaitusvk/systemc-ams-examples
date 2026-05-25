import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("eye_data.csv")

plt.figure(figsize=(8,5))

plt.scatter(
    df["time_ps"],
    df["voltage"],
    s=0.2
)

plt.xlabel("Time (ps)")
plt.ylabel("Voltage (V)")
plt.title("50 Gbps Eye Diagram")

plt.grid(True)

plt.show()