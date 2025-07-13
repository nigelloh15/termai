from openai import OpenAI

client = OpenAI(
    base_url="http://localhost:11434/v1",
    api_key="ollama",  # dummy but required
)

def getResponse(message):
    full_prompt = (
        "You are a terminal emulator in a Unix/Linux system. "
        "The user will ask for help with commands. "
        "Respond ONLY with a single line CLI command. "
        "Do NOT include explanations, code blocks, or multiple lines. "
        "If a command cannot be provided, respond with a very brief explanation why. "
        "For example, if the user asks for a command to list files, respond with only: ls\n\n"
        f"User query: {message}\n"
        "Response:"
    )

    response = client.chat.completions.create(
        model="deepseek-coder:6.7b",
        messages=[
            {"role": "system", "content": ""},  # empty or minimal system prompt
            {"role": "user", "content": full_prompt}
        ],
        temperature=0,
    )

    # Strip newlines and whitespace to ensure single line output
    return response.choices[0].message.content
