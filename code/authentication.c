#include "http_server.h"
#include <ctype.h>

static const unsigned char base64_decode_table[256] = {
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
    ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
    ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
    ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
    ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
    ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
    ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63};

// Funzione per decodificare una stringa Base64
int base64_decode(const char *src, char *out_buffer) {
    // Calcola la lunghezza della stringa di input
    int len = strlen(src);
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    // Decodifica la stringa Base64
    while (len-- && (src[in_] != '=') && (isalnum(src[in_]) || src[in_] == '+' || src[in_] == '/')) {
        // Riempie il buffer di 4 caratteri
        char_array_4[i++] = src[in_]; in_++;
        // Quando si hanno 4 caratteri, decodifica in 3 byte
        if (i == 4) {
            // Mappa i caratteri Base64 ai loro valori
            for (i = 0; i < 4; i++) char_array_4[i] = base64_decode_table[(int)char_array_4[i]];
            
            // Decodifica i 3 byte
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            // Scrive i 3 byte decodificati nell'output
            for (i = 0; (i < 3); i++) out_buffer[j++] = char_array_3[i];
            i = 0;
        }
    }

    // Gestisce i caratteri rimanenti
    if (i) {
        for (int k = i; k < 4; k++) char_array_4[k] = 0;

        for (int k = 0; k < 4; k++) char_array_4[k] = base64_decode_table[(int)char_array_4[k]];

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int k = 0; (k < i - 1); k++) out_buffer[j++] = char_array_3[k];
    }
    
    out_buffer[j] = '\0';
    return j;
}

int check_authentication(int client_fd, const char *buffer, const char *file_name, int flag){
    if (strncmp(file_name, "root/", 5) != 0 && flag == 0){
        return 1; // Cartella non protetta
    }


    const char *auth_header = strcasestr(buffer, "Authorization: Basic ");
    if (auth_header == NULL && flag == 1) return -1; 

    if (auth_header != NULL){
        // Estrai la stringa Base64
        const char *base64_creds = auth_header + strlen("Authorization: Basic ");
        char decoded_creds[128];
        
        // Trova la fine della stringa Base64
        char base64_string[128];
        int i = 0;
        while (base64_creds[i] != '\r' && base64_creds[i] != '\n' && base64_creds[i] != '\0' && i < 127) {
            base64_string[i] = base64_creds[i];
            i++;
        }
        base64_string[i] = '\0';

        // Decodifica
        if (base64_decode(base64_string, decoded_creds) == -1) return -1;

        // Confronta le credenziali
        char expected_creds[128];
        snprintf(expected_creds, sizeof(expected_creds), "%s:%s", "root", "toor");
        
        if (strcmp(decoded_creds, expected_creds) == 0) return 1;

        return -1;
    }    

    return -1; // Autenticazione fallita
}