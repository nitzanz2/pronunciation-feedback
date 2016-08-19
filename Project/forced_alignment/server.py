from bottle import get, post, request, route, run
import forced_alignment
#import model_builder
model_filename= "/opt/classification_server/models/best_so_far_gru_overlaps_3_labels.hdf5"
rnn_type='GRU'
model_built = False
current_model = None
request_number = 0
@get('/predict')
def predict():
    global model_built
    global current_model
    global request_number
    # print "In predict controller"
    # print "Request number %d" % request_number
    request_number = request_number + 1
    #if not model_built:
     #   current_model = model_builder.build_model(rnn_type, model_filename)
      #  model_built = True
    features_as_string = request.query.features
    labels = forced_alignment.main("example.wav", "example.phones", "example.TextGrid", False)
    #labels_output = repr(labels).replace('array','')
    #labels_output=''.join([c for c in labels_output if c not in ('(', ')','[',']',',')])
    #labels_output = labels_output.replace('\\n',';')
    scores_file_name = "final_scores.txt"
    scores_line_id = "normalized_phonem_confidences: "
    with open(scores_file_name, 'r') as f:
        for line in f:
            if scores_line_id in line:
                return line[len(scores_line_id):]
    #return "ok"
@get('/hello')
def hello():
    return "Hello!"
run(host='0.0.0.0', port=8081)