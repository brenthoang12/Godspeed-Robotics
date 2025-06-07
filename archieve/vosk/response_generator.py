import os
import json
from openai import OpenAI

def load_api_key():
    try:
        with open("eye_mechanism/.env") as f:
            for line in f:
                if line.strip().startswith("OPENAI_API_KEY="):
                    return line.strip().split("=", 1)[1].strip().strip('"\'')
    except FileNotFoundError:
        pass

    key = os.getenv("OPENAI_API_KEY")
    if not key:
        raise EnvironmentError("load_api_key(): key not found in .env or OS environment")
    
    return key



if __name__ == "__main__":
    client = OpenAI(api_key=load_api_key())


    response = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[{"role": "user", "content": "Hello!"}]
    )

    print(response.choices[0].message.content)