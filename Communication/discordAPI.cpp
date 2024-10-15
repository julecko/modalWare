#include <iostream>
#include <winsock2.h>
#include <iostream>
#include <string>
#include <random>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

//GLOBAL VARIABLES
std::string botToken;
SOCKET sock;
WSADATA wsaData;

void connect() {
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(443);
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed!" << std::endl;
        return;
    }

    std::cout << "Connected to Discord!" << std::endl;
}

void disconnect() {
    closesocket(sock);
    WSACleanup();
}
void sendMessage(const std::string& channelId, const std::string& message) {
    std::string url = "https://discord.com/api/v9/channels/" + channelId + "/messages";

    std::string request = "POST " + url + " HTTP/1.1\r\n"
        "Authorization: Bot " + botToken + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(message.length()) + "\r\n\r\n" +
        "{\"content\": \"" + message + "\"}";

    send(sock, request.c_str(), request.length(), 0);
}
std::string base64Encode(const std::vector<unsigned char>& input) {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string encoded;
    int val = 0;
    int valb = -6;

    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> valb) & 0x3F]);
    }

    while (encoded.size() % 4) {
        encoded.push_back('=');
    }

    return encoded;
}
std::string generateWebSocketKey() {
    std::vector<unsigned char> randomBytes(16);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned char> dis(0, 255);

    for (size_t i = 0; i < randomBytes.size(); ++i) {
        randomBytes[i] = dis(gen);
    }

    std::string base64Key = base64Encode(randomBytes);
    return base64Key;
}
void performHandshake(SOCKET sock, const std::string& botToken) {
    std::string key = generateWebSocketKey();

    std::string handshakeRequest = "GET /api/v9/gateway HTTP/1.1\r\n"
        "Host: discord.com\r\n"
        "Authorization: Bot " + botToken + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + key + "\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";

    send(sock, handshakeRequest.c_str(), handshakeRequest.length(), 0);

    // Read response
    char responseBuffer[4096];
    int bytesReceived = recv(sock, responseBuffer, sizeof(responseBuffer) - 1, 0);
    if (bytesReceived > 0) {
        responseBuffer[bytesReceived] = '\0';
        std::cout << "Response:\n" << responseBuffer << std::endl;

        if (strstr(responseBuffer, "HTTP/1.1 101 Switching Protocols")) {
            std::cout << "Handshake successful!" << std::endl;
        }
        else {
            std::cerr << "Handshake failed." << std::endl;
        }
    }
    else {
        std::cerr << "Failed to receive response." << std::endl;
    }
}
BOOL APIENTRY DllMain(HMODULE hModule,
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
}