# Generate a CSV of operations for the SSD sim
import csv
import argparse
import random

headers = ['op', 'die', 'plane', 'block', 'page']

parser = argparse.ArgumentParser(description="Generates traces for the SSD simulator")
parser.add_argument("filename", help="Output file name")
parser.add_argument("rows", help="number of rows to generate")
parser.add_argument("dies", help="num dies")
parser.add_argument("planes", help="num planes per die")
parser.add_argument("blocks", help="num blocks per plane")
parser.add_argument("pages", help="num pages per block") # blank for erase ops

args = parser.parse_args()

with open(args.filename, mode='w', newline='') as file:
	writer=csv.writer(file)
	writer.writerow(headers)

	for _ in range(int(args.rows)):

		op = random.choice(["eras", "prog", "read"])
		newrow = []

		die = random.randint(0, int(args.dies)-1)
		plane = random.randint(0, int(args.planes)-1)
		block = random.randint(0, int(args.blocks)-1)
		page = random.randint(0, int(args.pages)-1)

		if op == "erase":
			newrow = [op, die, plane, block, -1]
		else:
			newrow = newrow = [op, die, plane, block, page]
		writer.writerow(newrow)