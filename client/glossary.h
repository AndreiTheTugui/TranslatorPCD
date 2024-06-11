#ifndef GLOSSARY_H
#define GLOSSARY_H

#define MAXLINE 512

typedef struct
{
    char original[MAXLINE];
    char translated[MAXLINE];
} Translation;

Translation glossary[] = {
    {"hello", "salut"},
    {"world", "lume"},
    {"goodbye", "la revedere"},
    {"thank you", "mulțumesc"},
    {"please", "te rog"},
    {"yes", "da"},
    {"no", "nu"},
    {"time", "timp"},
    {"day", "zi"},
    {"night", "noapte"},
    {"morning", "dimineață"},
    {"afternoon", "după-amiază"},
    {"evening", "seară"},
    {"today", "astăzi"},
    {"tomorrow", "mâine"},
    {"yesterday", "ieri"},
    {"week", "săptămână"},
    {"month", "lună"},
    {"year", "an"},
    {"friend", "prieten"},
    {"family", "familie"},
    {"love", "dragoste"},
    {"work", "muncă"},
    {"home", "acasă"},
    {"school", "școală"},
    {"food", "mâncare"},
    {"drink", "băutură"},
    {"water", "apă"},
    {"coffee", "cafea"},
    {"tea", "ceai"},
    {"milk", "lapte"},
    {"bread", "pâine"},
    {"meat", "carne"},
    {"vegetable", "legume"},
    {"fruit", "fruct"},
    {"book", "carte"},
    {"pen", "pix"},
    {"paper", "hârtie"},
    {"computer", "calculator"},
    {"internet", "internet"},
    {"phone", "telefon"},
    {"music", "muzică"},
    {"movie", "film"},
    {"house", "casă"},
    {"car", "mașină"},
    {"bus", "autobuz"},
    {"train", "tren"},
    {"plane", "avion"},
    {"ticket", "bilet"},
    {"money", "bani"},
    {"price", "preț"},
    {"buy", "cumpărare"},
    {"sell", "vânzare"},
    {"eat", "mâncare"},
    {"drink", "băutură"},
    {"sleep", "somn"},
    {"wake up", "trezire"},
    {"clean", "curat"},
    {"wash", "spălat"},
    {"shower", "duș"},
    {"bath", "baie"},
    {"toilet", "toaletă"},
    {"chair", "scaun"},
    {"table", "masă"},
    {"bed", "pat"},
    {"window", "fereastră"},
    {"door", "ușă"},
    {"key", "cheie"},
    {"bag", "geantă"},
    {"book", "carte"},

};

int glossary_size = sizeof(glossary) / sizeof(glossary[0]);

#endif /* GLOSSARY_H */
