from bottle import get, post, request, route, run
import os
import subprocess
import forced_alignment
request_number = 0
@route('/', method='POST')
def predict():
    global request_number
    request_number = request_number + 1
    #get phones and record
    phones = request.forms.get('phones')
    wav= request.files.get('wav')
    wav.save("temp.wav")
    #change format
    s = ['sox','temp.wav', '-r', '16k','wavFile1.wav','remix','1',]
    subprocess.call(s)
    #remove silence
    s = ['sox','wavFile1.wav', 'temp1.wav','silence', '1', '0.01','1%','reverse']
    subprocess.call(s)
    s = ['sox','temp1.wav','wavFile.wav','silence', '1', '0.01','1%','reverse']
    subprocess.call(s)
    os.remove('temp1.wav')
    file = open("phones.phones", 'w')
    file.write(phones)
    file.close()
    forced_alignment.main("wavFile.wav", "phones.phones", "example.TextGrid", False)
    t=open("wavFile.wav",'r')
    os.remove("temp.wav")
    scores_file_name = "final_scores.txt"
    scores_line_id = "final_scores: "
    f2=open(scores_file_name, 'r')
    with open(scores_file_name, 'r') as f:
        for line in f:
            if scores_line_id in line:
                return line[len(scores_line_id):]
run(host='localhost', port=3000)

