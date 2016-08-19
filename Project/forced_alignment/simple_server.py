from bottle import get, post, request, route, run
#import model_builder
model_filename= "/opt/classification_server/models/best_so_far_gru_overlaps_3_labels.hdf5"
rnn_type='GRU'
model_built = False
current_model = None
request_number = 0
@get('/predict')
def predict():
    return "predict ok"
@get('/hello')
def hello():
    return "Hello!"
run(host='localhost', port=8081)
