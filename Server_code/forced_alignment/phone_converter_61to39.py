import sys
phones61_file_name = sys.argv[1]+'.phones'
phones39_file_name = sys.argv[1]+'39.phones'
with open(phones61_file_name,'r') as ph61,open('config/phoneme_map_timit61_to_leehon39','r') as map_file,open(phones39_file_name,'w') as ph39:
	origin_line = ph61.read();
	origin_phones = origin_line.split(' ')
	map = {}
	for line in map_file:
		mapping = line.split(' ')
		map[mapping[0]] = mapping[1][0:-1]
	#print origin_phones
	for phone in origin_phones:
		if '\n' in phone:
			phone = phone[0:-1]
		#print 'phone: ', phone
		if phone == 'sil':
			ph39.write('sil ')
		if phone in map:
			#print 'map[',phone,'] = ', map[phone]
			ph39.write(map[phone] + ' ')
				
	
