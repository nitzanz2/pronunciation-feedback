import wave, struct

waveFile = wave.open('out.wav', 'r')
name=raw_input('file name ')
word=raw_input('word ')

phn=name+".phn"
wrd=name+".wrd"
phones=word+".phones"
wav=word+".wav"
output_file = wave.open(wav, 'wb')
phn_file=open(phn, "r")
file2 = open(phones, "w")
wrd_file  = open(wrd, "r")


string=wrd_file.read()
string=string.splitlines()
phns=phn_file.read()
phns=phns.splitlines()

print string
lines=[]
for s in string:
    lines.append(s.split())
print lines
f=0
t=0
flag=0
for l in lines:
    if(word in l):
        f=int(l[0])
        t=int(l[1])
        flag=1
if(flag==0):
    print "error"
phones_s=""
flag1=0
for p in phns:
    temp=p.split()
    print temp
    if(flag1==1):
        phones_s=phones_s+" "+temp[2]
    if (int(temp[0])==f):
        phones_s=phones_s+temp[2]
        flag1=1
    if(int(temp[1])==t):
        flag1=0
file2.write(phones_s)
#Get input file parameters and set them to the output file after modifing the channel number.
in_params = list(waveFile.getparams())

out_params = in_params[:]
out_params[0] = 1
output_file.setparams(out_params)

nchannels, sampwidth, framerate, nframes, comptype, compname = in_params
format = '<{}h'.format(nchannels)



length = waveFile.getnframes()
list=[]
for i in range(0,length):
    waveData = waveFile.readframes(1)
    data = struct.unpack("<h", waveData)
    #print int(data[0])
    list.append(data[0])
#print list
#print len(list)
#print length



out=list[f:t]

#print len(out)
for i in range (len(out)):
    output_file.writeframes(struct.pack("<h", out[i]))