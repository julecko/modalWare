#include <iostream>
#include <string>
#include <curl/curl.h>

const std::string DISCORD_API_URL = "https://discord.com/api/v10/channels/1146741461614870599/messages";
const std::string BOT_TOKEN = "DISCORD_BOT_TOKEN";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendMessage(const std::string& message) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string url = DISCORD_API_URL;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bot " + BOT_TOKEN).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string json = "{\"content\":\"" + message + "\"}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}


/*BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        const char* token = "DISCORD_BOT_TOKEN";
        //isAuthorized = InitializeBot(token);
        break;
    case DLL_PROCESS_DETACH:
        // Cleanup if necessary
        break;
    }
    return TRUE;
}*/