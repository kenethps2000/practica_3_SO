#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

char* str_replace(const char* str, const char* old, const char* new_sub) {
    if (!str || !old || !new_sub) return NULL;

    size_t old_len = strlen(old);
    size_t new_len = strlen(new_sub);
    size_t str_len = strlen(str);

    // Estimación del tamaño máximo de la cadena resultante
    size_t result_len = str_len;
    for (size_t i = 0; i < str_len; i++) {
        if (strncmp(&str[i], old, old_len) == 0) {
            result_len = result_len - old_len + new_len;
        }
    }

    // Reservar espacio para la nueva cadena
    char *result = (char*)malloc(result_len + 1);
    if (!result) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < str_len;) {
        if (strncmp(&str[i], old, old_len) == 0) {
            strcpy(&result[j], new_sub);
            j += new_len;
            i += old_len;
        } else {
            result[j++] = str[i++];
        }
    }
    result[j] = '\0';
    return result;
}

// Función para enviar el mensaje de WhatsApp
void enviar_mensaje(const char *numero_destino, const char *mensaje) {
    CURL *curl;
    CURLcode res;
    const char *sid = "";
    const char *token = "";
    const char *from_number = "";
    // Formatear el número de destino
    char* numero_destino_formateado = str_replace(numero_destino, "+", "%2B");
    char* numero_formateado = str_replace(from_number, "+", "%2B");
    if (!numero_destino_formateado) {
        fprintf(stderr, "Error al formatear el número de destino.\n");
        return;
    }

    // Inicializar cURL
    curl = curl_easy_init();
    if (curl) {
        // Formatear la URL de Twilio
        char url[256];
        snprintf(url, sizeof(url), "https://api.twilio.com/2010-04-01/Accounts/%s/Messages.json", sid);

        // Configurar los campos del formulario
        char postfields[512];
        snprintf(postfields, sizeof(postfields),
                 "To=%s&From=%s&Body=%s",
                 numero_destino_formateado, numero_formateado, mensaje);

        // Configurar las opciones de cURL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

        // Autenticación básica
        char auth[128];
        snprintf(auth, sizeof(auth), "%s:%s", sid, token);
        curl_easy_setopt(curl, CURLOPT_USERPWD, auth);

        // Realizar la solicitud
        res = curl_easy_perform(curl);
        // Comprobar errores
        if (res != CURLE_OK) {
            fprintf(stderr, "Error al enviar el mensaje: %s\n", curl_easy_strerror(res));
        } else {
            printf("Mensaje enviado correctamente.\n");
        }

        // Limpiar
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Error al inicializar cURL.\n");
    }

    free(numero_destino_formateado);
    free(numero_formateado);
}
