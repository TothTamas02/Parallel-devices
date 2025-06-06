import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read CSV
df = pd.read_csv("measurements.csv")
df = df.drop_duplicates()

# Calculate average running times by filter and device
avg_times = df.groupby(['filter', 'device_name'])['elapsed_time(ms)'].mean().reset_index()

# Filter unique values for x-axis
filters = sorted(df['filter'].unique())
x = np.arange(len(filters))

# Get devices and calculate bar width
devices = df["device_name"].unique()
bar_width = 0.8 / len(devices)

# Create figure
plt.figure(figsize=(15, 8))

# Calculate y-axis limit
all_values = avg_times['elapsed_time(ms)'].values
max_value = np.max(all_values)
y_limit = max_value * 0.02  # Upper limit for y-axis

# Clean device names (remove underscores)
clean_device_names = [name.replace('_', ' ') for name in devices]
device_name_map = dict(zip(devices, clean_device_names))

# Create a bar for each device
for i, device in enumerate(devices):
    device_data = avg_times[avg_times['device_name'] == device]
    y = [device_data[device_data['filter'] == f]['elapsed_time(ms)'].values[0]
         if not device_data[device_data['filter'] == f].empty else 0
         for f in filters]

    positions = [pos + i * bar_width for pos in x]

    # Draw bars with automatic colors, use clean names for legend
    bars = plt.bar(positions, y, width=bar_width, label=device_name_map[device])

    # Add values to the bars
    for j, bar in enumerate(bars):
        height = y[j]
        if height > 0:  # Only if there's a value
            # Check if the bar overflows
            if height > y_limit:
                # For overflowing bars, write value horizontally
                plt.text(bar.get_x() + bar.get_width()/2, y_limit * 0.9,
                         f"{height:.1f}", ha='center', va='top',
                         rotation=90, color='black', fontweight='bold', fontsize=9)
            else:
                # For small bars, write value on top
                plt.text(bar.get_x() + bar.get_width()/2, height + max_value*0.001,
                         f"{height:.1f}", ha='center', va='bottom',
                         color='black', fontsize=9)

# Clean filter names (remove underscores)
clean_filters = [f.replace('_', ' ') for f in filters]

# X-axis settings
plt.xticks(x + bar_width, clean_filters)
plt.ylabel("Average execution time (ms)")
plt.xlabel("Filter")
plt.title("Average filter execution times by device")
plt.legend(title="Device name", loc='upper right')
plt.grid(True, axis="y", linestyle='--', alpha=0.7)

# Y-axis limitation
plt.ylim(0, y_limit)

# Better appearance
plt.tight_layout()

# Save and display
plt.savefig("graph.jpg", dpi=300)
plt.show()
