#include "pro_os.h"
#include <string.h>

static char current_lang[8] = "en";

typedef struct {
    const char *key;
    const char *en;
    const char *fr;
    const char *de;
} translation_entry_t;

static translation_entry_t translations[] = {
    {"welcome", "welcome back user!", "bonjour utilisateur!", "willkommen benutzer!"},
    {"login", "Login", "Connexion", "Anmelden"},
    {"password", "password", "mot de passe", "passwort"},
    {"power", "Power", "Puissance", "Strom"},
    {"desktop", "Desktop", "Bureau", "Desktop"},
    {"settings", "Settings", "Paramètres", "Einstellungen"},
    {"explorer", "Explorer", "Explorateur", "Explorer"},
    {"notepad", "Notepad", "Bloc-notes", "Notepad"},
    {"calculator", "Calculator", "Calculatrice", "Taschenrechner"},
    {"terminal", "Terminal", "Terminal", "Terminal"},
    {"taskmgr", "Task Manager", "Gestionnaire de tâches", "Task-Manager"},
};

const char* i18n_translate(const char *key) {
    int count = sizeof(translations) / sizeof(translations[0]);
    for (int i = 0; i < count; i++) {
        if (strcmp(translations[i].key, key) == 0) {
            if (strcmp(current_lang, "fr") == 0) return translations[i].fr;
            if (strcmp(current_lang, "de") == 0) return translations[i].de;
            return translations[i].en;
        }
    }
    return key;
}

void i18n_set_language(const char *lang) {
    strncpy(current_lang, lang, 8);
}

const char* i18n_get_language(void) {
    return current_lang;
}
