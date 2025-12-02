import os
import google.generativeai as genai
from github import Github

# --- CONFIGURAZIONE ---
GOOGLE_API_KEY = "AIzaSyA-N5Ip1RiYfEtz_amIm4ulzqM5Re85lWQ"
GITHUB_TOKEN = "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"
REPO_NAME = "hardwayy/house" # Es: "MarioRossi/HouseProject"

# Estensioni da leggere (Filtro per scalabilità: ignoriamo i binari e i file inutili)
ALLOWED_EXTENSIONS = {'.h', '.cpp', '.cs'} 
# Cartelle da ignorare (Fondamentale per Unreal)
IGNORED_DIRS = {'Intermediate', 'Binaries', 'Saved', 'Build', '.git', '.vs'}

# --- SETUP SYSTEM INSTRUCTION (Dal prompt precedente) ---
SYSTEM_PROMPT = """
Sei un Senior Software Engineer esperto e un mentore tecnico. 
Il tuo obiettivo è assistere l'utente basandoti ESCLUSIVAMENTE sull'analisi del codice presente nel commit più recente.
[... INCOLLA QUI TUTTA LA DESCRIZIONE CHE TI HO DATO PRIMA ...]
"""

def get_repo_content(repo_name):
    """Scarica ricorsivamente solo i file sorgente dall'ultimo commit."""
    print(f"📡 Connessione a GitHub: {repo_name}...")
    g = Github(GITHUB_TOKEN)
    repo = g.get_repo(repo_name)
    
    contents = repo.get_contents("")
    code_context = ""
    file_count = 0

    while contents:
        file_content = contents.pop(0)
        
        # Ignora cartelle proibite
        if file_content.type == "dir":
            if file_content.name not in IGNORED_DIRS:
                contents.extend(repo.get_contents(file_content.path))
        
        # Leggi solo file sorgente
        else:
            _, ext = os.path.splitext(file_content.name)
            if ext in ALLOWED_EXTENSIONS:
                try:
                    # Decodifica il contenuto
                    raw_code = file_content.decoded_content.decode('utf-8')
                    code_context += f"\n--- FILE: {file_content.path} ---\n{raw_code}\n"
                    file_count += 1
                except:
                    print(f"⚠️ Impossibile leggere: {file_content.path}")

    print(f"✅ Letti {file_count} file sorgente dall'ultimo commit.")
    return code_context

def main():
    genai.configure(api_key=GOOGLE_API_KEY)

    # 1. Scarica il codice fresco
    code_base = get_repo_content(REPO_NAME)

    # 2. Configura il Modello
    model = genai.GenerativeModel(
        model_name="gemini-1.5-pro-latest", # Usa il Pro per avere una finestra di contesto grande
        system_instruction=SYSTEM_PROMPT
    )

    # 3. Avvia la chat iniettando il codice come primo messaggio (invisibile all'utente)
    # Questo tecnica si chiama "Context Stuffing"
    initial_context = f"Ecco il codice sorgente attuale del progetto (Ultimo Commit):\n{code_base}\n\nAttendo istruzioni."
    
    chat = model.start_chat(history=[
        {"role": "user", "parts": [initial_context]},
        {"role": "model", "parts": ["Codice ricevuto e analizzato. Sono pronto. Qual è il prossimo passo?"]}
    ])

    print("\n🤖 GitMentor è online. Scrivi 'refresh' per ricaricare il codice da GitHub o 'exit' per uscire.\n")

    while True:
        user_input = input("Tu: ")
        if user_input.lower() == 'exit':
            break
        elif user_input.lower() == 'refresh':
            # Ricarica manuale se hai fatto un push mentre parlavi
            print("🔄 Ricaricamento repository...")
            new_code = get_repo_content(REPO_NAME)
            # Reimpostiamo la chat con il nuovo contesto
            chat = model.start_chat(history=[
                {"role": "user", "parts": [f"AGGIORNAMENTO CODICE:\n{new_code}"]},
                {"role": "model", "parts": ["Ho aggiornato il mio contesto con il nuovo codice."]}
            ])
            continue

        response = chat.send_message(user_input)
        print(f"GitMentor: {response.text}")

if __name__ == "__main__":
    main()