#!/usr/bin/python
# Compute AUC
# syntax: arg1 = pos scores, arg2 = neg scores

import sys

pos = sys.argv[1]
neg = sys.argv[2]

fp = open(pos, 'r')
pos_s = fp.readlines()
pos_s = [float(i) for i in pos_s]
fn = open(neg, 'r')
neg_s = fn.readlines()
neg_s = [float(i) for i in neg_s]

total = 0.0
for i in pos_s:
 for j in neg_s:
	if (i > j): total += 1.0
total /= len(pos_s) * len(neg_s) 

print total
