import os
import wave
import json
from vosk import Model, KaldiRecognizer

# Load model
model = Model(model_path="eye_mechanism/models/vosk-model-en-us-0.22-lgraph")

# Open WAV file
wf = wave.open("eye_mechanism/samples/jfk.wav", "rb")
rec = KaldiRecognizer(model, wf.getframerate())

# Read and transcribe
results = []
while True:
    data = wf.readframes(4000)
    if len(data) == 0:
        break
    if rec.AcceptWaveform(data):
        print(rec.Result())
        # results.append(json.loads(rec.Result()))
        

# Final result
results.append(json.loads(rec.FinalResult()))
for r in results:
    print(r.get("text", ""))

wf.close()
