import os
import whisper

os.environ['SSL_CERT_FILE'] = '/etc/ssl/cert.pem' #security credential for python 3.12

model = whisper.load_model("base")

result = model.transcribe("data/harvard.wav", language="en")

print("Transcription:", result["text"])
