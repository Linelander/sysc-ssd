import pandas as pd
import matplotlib
import argparse

parser = argparse.ArgumentParser(description="Analyzes SSD model output")
parser.add_argument("input", help="file to analyze")
args = parser.parse_args()

df = pd.read_csv(args.input)

# --- Warm-up ---
# 1. Print the total number of operations, and the count of each op type.
print()
print("Total ops: " + str(df["op"].count()))
tot_by_type = df.groupby("op")["op"].count()
print(tot_by_type.to_string())

# 2. Print the number of failures per op type.
print()
print("Fails per type")
mask = df["latency_us"].isna()
failed = df[mask].groupby("op")["op"].count()
print(failed.to_string())

# 3. Print the failure rate (%) per op type.
print()
print(round((failed / tot_by_type) * 100, 2).to_string())

# --- Filtering ---
# 4. Make a DataFrame of only successful operations. Use it for the rest.
sdf = df[df["latency_us"].notna()]

# 5. How many reads succeeded on die 0, plane 1?
print()
rsdf = sdf[sdf["op"] == "read"]
dmask = rsdf["die"] == 0
plmask = rsdf["plane"] == 1
dp_rsdf = rsdf[dmask & plmask]
print("Successful reads on die 0, plane 1: " + str(len(dp_rsdf)))

# 6. Print the 5 operations with the latest completion time.
print()
print("5 operations with the latest completion time")
print(sdf.sort_values("completed_us").tail(5).to_string())

# --- Grouping ---
# 7. Average latency per die, broken down by op type.
print()
print("Average latency per die, broken down by op type.")
print(sdf.groupby(["die", "op"])["latency_us"].mean().to_string()) # can group by 2 things

# 8. Erase count per block (die, plane, block). Which block was erased most?
print()
print("erase count per block")
e_perbl_ct = sdf[sdf["op"] == "eras"].groupby(["die", "plane", "block"]).size()
print(e_perbl_ct.sort_values().tail(1).to_string())


# 9. Number of blocks that were never erased.
#    (Hint: you know how many blocks exist from the config.)
# to ask about values, compare series to itself
print()
print("blocks never erased")
print(2*2*16 - len(e_perbl_ct)) # because e_perbl_ct is the blocks that WERE erased

# --- Derived columns ---
# 10. Add a service_us column (prog=40, read=10, eras=1000) and a
#     queue_us column = latency_us - service_us. Confirm queue_us is all 0.
sdf["service_us"] = sdf["op"].map({"prog": 40, "read": 10, "eras": 1000}) 
# print(sdf.to_string())
sdf["queue_us"] = sdf["latency_us"] - sdf["service_us"]
# print(sdf[["op", "queue_us"]].to_string())
# 11. Total simulated runtime: last completion minus first issue.
print()
print("Total runtime: " + str(sdf.tail(1)["completed_us"].iloc[0] - sdf.head(1)["issued_us"].iloc[0]) + " microseconds")

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