#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import json
import queue
import sounddevice as sd
from vosk import Model, KaldiRecognizer

MODEL_PATH = "../models/vosk-model-small-cn-0.22"

q = queue.Queue()

def callback(indata, frames, time, status):
    if status:
        print(status, file=sys.stderr)
    q.put(bytes(indata))

def main():
    try:
        model = Model(MODEL_PATH)
        recognizer = KaldiRecognizer(model, 16000)

        print("请开始说话，例如：明天下午三点提醒我写作业", file=sys.stderr)

        with sd.RawInputStream(
            samplerate=16000,
            blocksize=8000,
            dtype='int16',
            channels=1,
            callback=callback
        ):
            # 最多录音 6 秒
            for _ in range(0, 60):
                data = q.get()
                if recognizer.AcceptWaveform(data):
                    break

        result = json.loads(recognizer.FinalResult())
        text = result.get("text", "").replace(" ", "")

        print(text)

    except Exception as e:
        print("", file=sys.stdout)
        print("语音识别错误:", e, file=sys.stderr)

if __name__ == "__main__":
    main()
