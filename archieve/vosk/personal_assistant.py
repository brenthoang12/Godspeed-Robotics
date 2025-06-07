# TODO: implement end-of-user turn
# TODO: implement punctuation package
# TODO: implement ChatGPT response
# TODO: make this as a library?

import sys
import json
import sounddevice as sd
from vosk import Model, KaldiRecognizer

# constants 
MODEL_PATH = "eye_mechanism/models/vosk-model-en-us-0.22-lgraph"  
SAMPLE_RATE = 16000

def load_recognizer(model_path, sample_rate):
    model = Model(model_path)
    recognizer = KaldiRecognizer(model, sample_rate)
    return recognizer

def transcribe_callback(recognizer):
    def callback(indata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        if recognizer.AcceptWaveform(bytes(indata)):
            result = recognizer.Result()
            text = json.loads(result).get("text", "")
            if text.strip():
                print("You:", text) 
    return callback

def start_real_time_transcription(model_path, sample_rate):
    rec = load_recognizer(model_path, sample_rate)
    callback = transcribe_callback(rec)
    with sd.RawInputStream(samplerate=SAMPLE_RATE, blocksize=8000, dtype='int16',
                        channels=1, callback=callback):
        print("Speak into your microphone (Ctrl+C to stop)...")
        try:
            while True:
                pass
        except KeyboardInterrupt:
            print("\nTranscription stopped.")



if __name__ == "__main__":
    start_real_time_transcription(MODEL_PATH, SAMPLE_RATE)
