#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
import random

numParam = len(sys.argv)

if numParam != 3:
	print("Usage: python simuEvaluator fullEval? individualFile");
else:
	goFull = bool(int(sys.argv[1]))
	individualFile = sys.argv[2]
	
	#print('Go full?: {} ; Input Filename: {}'.format(goFull, individualFile)) 
	
	f = open(individualFile, 'r')
	components = f.readlines()[0].split()
	f.close()	

	sumComps = 0.0
	for i in components:
		sumComps = sumComps + float(i)
	intSumComps = int(sumComps // 1) # The integer part of sumComps	

	indValue = -1.0 # Hypothetical individual evaluation

	if (intSumComps % 2 == 0): # Feasible
		if goFull:
			#print('Using a GPU to process {}'.format(individualFile))
			indValue = random.random()*6.0 # Random number in [0, 6)
		# else -> Do not do nothing, leave the individual PENDING!
	else:		# Infeasible
		indValue = 500.0 + random.random()*500.0  # Infeasible evaluation process aimed at CPU

	print(indValue) 
	# NOTE: Or write this to another file for the Oracle to read... <Assuming direct output for simplicity>
