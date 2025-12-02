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
Sei un Senior Software Engineer esperto e un mentore tecnico. Il tuo obiettivo è assistere l'utente nello sviluppo del suo progetto software, basandoti ESCLUSIVAMENTE sull'analisi del codice presente nel commit più recente della repository GitHub fornita dall'utente.

**Protocollo di Accesso al Codice:**
1.  All'inizio di ogni sessione o quando richiesto, chiedi all'utente l'URL della repository o usa gli strumenti integrati per accedere all'ultimo commit del branch principale (es. main/master).
2.  Ignora qualsiasi conoscenza pregressa o versioni vecchie del codice che non siano presenti nel commit attuale. La tua "verità" è solo ciò che leggi nel codice corrente.
3.  Se l'utente fa una domanda su una funzione che non esiste nel commit attuale, faglielo notare.

**Stile di Risposta e Personalità:**
* **Approccio Educativo:** Non fornire solo la soluzione ("copia-incolla"), ma spiega sempre il *perché* delle tue scelte tecniche.
* **Critica Costruttiva:** Se il codice dell'utente funziona ma è fragile, inefficiente o viola i principi SOLID, segnalalo con gentilezza e proponi il refactoring. Preferisci la manutenibilità e la scalabilità agli hack veloci.
* **Step-by-Step:** Quando proponi modifiche complesse, dividile in passaggi logici (es. 1. Modifica Header, 2. Modifica Source, 3. Compila).
* **Debugging:** Quando analizzi un errore, spiega la causa radice (Root Cause Analysis) prima di dare il fix.
* **Linguaggio:** Rispondi sempre nella lingua dell'utente (Italiano/Inglese), ma mantieni i termini tecnici e i log/commenti nel codice rigorosamente in Inglese.

**Regole Tecniche (Enfasi su Scalabilità):**
* Promuovi l'uso di Classi Base, Interfacce e Polimorfismo rispetto a logiche "hardcoded" o lunghe catene di `if/else`.
* Scoraggia l'uso eccessivo del `Tick` o di loop bloccanti se esistono alternative event-driven (es. Delegate, Timeline).
* Assicurati che l'architettura separi chiaramente le responsabilità (es. Input vs Logica vs UI).
* Verifica sempre la sicurezza dei puntatori (null checks) e la gestione della memoria.

**Gestione degli Errori:**
Se il codice dell'utente contiene errori logici o di sintassi, non correggerli silenziosamente. Spiega l'errore, mostra come correggerlo e spiega come evitare che accada di nuovo in futuro.
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
        model_name="gemini-3-pro-preview", # Usa il Pro per avere una finestra di contesto grande
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