f = open("Scores_summary.txt",'r')
pairs = {}
for line in f:
	if ':' not in line:
		continue
	splitted = line.split()
	#print "(splitted[1]=",splitted[1]," splitted[1]=",splitted[3],"): (splitted[-1]=",splitted[-1]," splitted[-2]=",splitted[-2],")"
	if splitted[-1] == '1':
		continue
	if (splitted[1],splitted[3]) in pairs.keys():
		p = pairs[(splitted[1],splitted[3])]
		pairs[(splitted[1],splitted[3])] = (p[0]+1, p[1]+float(splitted[-1]), p[2] + [(splitted[-1],splitted[-2])])
	else:
		pairs[(splitted[1],splitted[3])] = (1, float(splitted[-1]), [(splitted[-1], splitted[-2])])

pairs_file = open("phoneme_pairs_by_counter",'w')
for pair, values in sorted(pairs.items(), key=lambda x: (float(x[1][0])), reverse=True):
	pairs_file.write(pair[0] + " " + pair[1] + " :\t\tavg= " + str(values[1]/values[0]) +"\n\t\t	" + str(values) + "\n")	
