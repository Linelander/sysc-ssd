import pandas as pd
import matplotlib
import argparse

parser = argparse.ArgumentParser(description="Analyzes SSD model output")
parser.add_argument("input", help="file to analyze")
args = parser.parse_args()

df = pd.read_csv(args.input)

# --- Warm-up ---
# 1. Print the total number of operations, and the count of each op type.
print("Total ops: " + str(df["op"].count()))
print(df.groupby("op")["op"].count().to_string())
# 2. Print the number of failures per op type.
print()
print("Fails per type")
print(df.groupby("op")["latency_us"].isna().sum())

# 3. Print the failure rate (%) per op type.

# --- Filtering ---
# 4. Make a DataFrame of only successful operations. Use it for the rest.
# 5. How many reads succeeded on die 1, plane 0?
# 6. Print the 5 operations with the latest completion time.

# --- Grouping ---
# 7. Average latency per die, broken down by op type.
# 8. Erase count per block (die, plane, block). Which block was erased most?
# 9. Number of blocks that were never erased.
#    (Hint: you know how many blocks exist from the config.)

# --- Derived columns ---
# 10. Add a service_us column (prog=40, read=10, eras=1000) and a
#     queue_us column = latency_us - service_us. Confirm queue_us is all 0.
# 11. Total simulated runtime: last completion minus first issue.
# 12. Per-die utilization: busy time (sum of service_us) / total runtime.

# --- Throughput ---
# 13. Operations completed per millisecond, bucketed by completion time.
#     (Hint: integer-divide completed_us by 1000, then group.)

# --- Plots (save to results/, don't show) ---
# 14. Bar chart: successful ops per op type.
# 15. Bar chart: utilization per die.
# 16. Histogram of erase counts per block.
# 17. Line plot: throughput over time from #13.

# --- Stretch ---
# 18. Throughput-weighted write amplification is N/A here, but compute
#     "bytes programmed" assuming 16 KiB pages, and report MB/s of
#     program bandwidth over the run.
# 19. Make the script take an optional --op flag that limits every
#     stat above to one op type.