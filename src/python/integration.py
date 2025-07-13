import site
from openai import OpenAI

client = OpenAI(
    base_url="http://localhost:11434/v1",
    api_key="ollama"  # dummy but required
)

def getResponse(message):
    response = client.chat.completions.create(
        model="deepseek-coder:6.7b",
        messages=[
            {
                "role": "system",
                "content": (
                    "You are a terminal emulator. The user will ask for help with commands. "
                    "You must respond with ONLY a single line CLI command. Do not include explanations, "
                    "code blocks, or multiple lines. Only return one line of terminal command output."
                    "For example, if the user asks for a command to list files, repond with only 'ls'"
                )
            },
            {"role": "user", "content": message}
        ],
    )

    # Strip newlines and excessive whitespace to ensure it's one clean line
    return response.choices[0].message.content
