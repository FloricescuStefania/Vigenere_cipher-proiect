#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <mpi.h>

char shiftChar(unsigned char c, unsigned char k, bool encrypt) {

    if (!isalpha(c)) return c;

    //transformare in litera
    char P = toupper(c);
    char K = toupper(k);
    int Pi = P - 'A';
    int Ki = K - 'A';
    int res = encrypt ? (Pi + Ki) % 26 : (Pi - Ki + 26) % 26;

    return (char)(res + 'A');
}

std::string runMPIVigenere(const std::string& input, const std::string& key, bool encrypt, int rank, int world_size) {

    int total_chars = input.length();
    //procesul 0 trimite tot textul la celelalte procese
    MPI_Bcast(&total_chars, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = total_chars / world_size;
    std::vector<char> local_chunk(chunk_size);

    //trimitem bucata la fiecare proces
    MPI_Scatter(input.data(), chunk_size, MPI_CHAR,local_chunk.data(), chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    for (int i = 0; i < chunk_size; ++i) {
        int global_idx = rank * chunk_size + i;
        local_chunk[i] = shiftChar(local_chunk[i], key[global_idx % key.length()], encrypt);
    }

    std::string result = "";
    if (rank == 0) result.resize(total_chars);

    //refacem textul
    MPI_Gather(local_chunk.data(), chunk_size, MPI_CHAR,&result[0], chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    return result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::string key;
    std::string original_text, encrypted, decrypted;
    int key_len;

    if (rank == 0) {
        std::cout << "Enter key: ";
        std::cin >> key;
        key_len = key.length();
    }

    //fiecare proces primeste cheia primita
    MPI_Bcast(&key_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) key.resize(key_len);

    MPI_Bcast(&key[0], key_len, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        std::ifstream file("Beauty_And_The_Beast_20MB.txt");
        if (!file) {
            std::cerr << "Error: Could not open file!\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        std::string line;
        while (getline(file, line)) original_text += line + "\n";
        file.close();

        while (original_text.length() % world_size != 0) original_text += ' ';

    }

    auto startEnc = std::chrono::high_resolution_clock::now();
    encrypted = runMPIVigenere(original_text, key, true, rank, world_size);
    auto endEnc = std::chrono::high_resolution_clock::now();

    if (rank == 0) {
        std::ofstream out("encrypted.txt");
        out << encrypted;
        out.close();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(endEnc - startEnc);
        std::cout << "Encryption time: " << dur.count() << " ms\n";
    }

    auto startDec = std::chrono::high_resolution_clock::now();
    decrypted = runMPIVigenere(encrypted, key, false, rank, world_size);
    auto endDec = std::chrono::high_resolution_clock::now();

    if (rank == 0) {
        std::ofstream out("decrypted.txt");
        out << decrypted;
        out.close();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(endDec - startDec);
        std::cout << "Decryption time: " << dur.count() << " ms\n";
    }

    MPI_Finalize();
    return 0;
}