#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <chrono>
#include <thread>
#include <vector>

class VigenereCipher
{
private:
    std::string key;

    //mareste lungimea cheii
    std::string generateFullKey(const std::string& text) const
    {
        std::string fullKey = key;
        while (fullKey.length() < text.length()) {
            fullKey += key;
        }
        return fullKey.substr(0, text.length());
    }

    //functie ajutatoare pentru criptare/decriptare
    char shiftChar(unsigned char c, unsigned char k, bool encrypt = true) const
    {
        if (!isalpha(c)) return c;

        char P = toupper(c);
        char K = toupper(k);

        int Pi = P - 'A';
        int Ki = K - 'A';

        int Rez;

        if (encrypt)
            Rez = (Pi + Ki) % 26;
        else
            Rez = (Pi - Ki + 26) % 26;

        return char(Rez + 'A');
    }

    //bucata pentru fiecare thread
    //criptarea si decriptarea scrisa in result
    void processChunk(const std::string& text, const std::string& fullKey,std::string& result, size_t start, size_t end, bool encrypt) const
    {
        for (size_t i = start; i < end; i++)
        {
            result[i] = shiftChar(text[i], fullKey[i], encrypt);
        }
    }

    //procesare paralela
    std::string processParallel(const std::string& text, bool encrypt) const
    {
        size_t n = text.length();
        std::string result(n, '\0');

        //nr de thread-uri luate din 
        size_t numThreads = std::thread::hardware_concurrency();
        //daca nu se da nr de thread-uri setam implicit la 4
        if (numThreads == 0) numThreads = 4;

        std::vector<std::thread> threads(numThreads);

        //generam cheia
        std::string fullKey = generateFullKey(text);

        size_t chunkSize = n / numThreads;

        for (size_t t = 0; t < numThreads; t++)
        {
            //start si end pt fiecare bucata
            size_t start = t * chunkSize;
            size_t end;
            if (t == numThreads - 1)
                end = n; 
            else
                end = start + chunkSize;

  
            threads[t] = std::thread([this, &text, &fullKey, &result, start, end, encrypt]() {processChunk(text, fullKey, result, start, end, encrypt);});
        }

        //asteptam ca toate thread-urile sa termine
        for (auto& th : threads)
            th.join();

        return result;
    }

public:
    VigenereCipher(const std::string& k) : key(k) {}

    //criptare
    std::string encrypt(const std::string& text) const
    {
        return processParallel(text, true);
    }

    //decriptare
    std::string decrypt(const std::string& text) const
    {
        return processParallel(text, false);
    }
};

//citire din fisier
std::string readFile(const std::string& filename)
{
    std::ifstream file(filename);
    std::string content, line;
    while (getline(file, line))
        content += line + "\n";
    file.close();
    return content;
}

//scriere in fisier
void writeFile(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    file << content;
    file.close();
}

int main()
{
    std::string key;
    std::cout << "Introduceti cheia: ";
    std::cin >> key;

    std::cout << "Thread-uri disponibile: " << std::thread::hardware_concurrency() << "\n";

    std::string text = readFile("Beauty_And_The_Beast_100KB.txt");

    VigenereCipher cipher(key);

    //masuram timpul pentru criptare
    auto startEnc = std::chrono::high_resolution_clock::now();
    std::string encrypted = cipher.encrypt(text);
    auto endEnc = std::chrono::high_resolution_clock::now();

    writeFile("encrypted.txt", encrypted);

    auto durationEnc = std::chrono::duration_cast<std::chrono::milliseconds>(endEnc - startEnc);
    std::cout << "Timp criptare: " << durationEnc.count() << " ms\n";

    //masuram timpul pentru decriptare
    auto startDec = std::chrono::high_resolution_clock::now();
    std::string decrypted = cipher.decrypt(encrypted);
    auto endDec = std::chrono::high_resolution_clock::now();

    writeFile("decrypted.txt", decrypted);

    auto durationDec = std::chrono::duration_cast<std::chrono::milliseconds>(endDec - startDec);
    std::cout << "Timp decriptare: " << durationDec.count() << " ms\n";

    return 0;
}