#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

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
        return fullKey.substr(0, text.length());//daca depaseste se taie cat este textul
    }

    //functie ajutatoare pentru criptare/decriptare
    char shiftChar(unsigned char c, unsigned char k, bool encrypt = true) const
    {
        //verificam daca este litera
        if (!isalpha(c)) return c;

        //transformam caracterele in majuscule
        char P = toupper(c); // P-caracterul din text
        char K = toupper(k); // K-caracterul din cheie

        int Pi = P - 'A'; // index 0-25 pentru text
        int Ki = K - 'A'; // index 0-25 pentru cheie

        int Rez;

        if (encrypt) 
        {
            // Criptare: Ci = (Pi + Ki) mod 26
            Rez = (Pi + Ki) % 26;
        }
        else 
        {
            // Decriptare: Di = (Ci - Ki + 26) mod 26
            Rez = (Pi - Ki + 26) % 26;
        }

        return char(Rez + 'A'); // convertim inapoi la litera
    }

public:
    VigenereCipher(const std::string& key) : key(key) {}

    //criptare
    std::string encrypt(const std::string& text) const 
    {
        std::string fullKey = generateFullKey(text);
        std::string result = "";
        //parcurge textul, aplica functia si concateneaza intr-un rezultat
        for (size_t i = 0; i < text.length(); i++) 
        {
            result += shiftChar(text[i], fullKey[i], true);
        }
        return result;
    }

    //decriptare
    std::string decrypt(const std::string& text) const 
    {
        std::string fullKey = generateFullKey(text);
        std::string result = "";
        for (size_t i = 0; i < text.length(); i++) 
        {
            result += shiftChar(text[i], fullKey[i], false);
        }
        return result;
    }
};

//citire din fisier
std::string readFile(const std::string& filename) 
{
    std::ifstream file(filename);
    std::string content, line;
    while (getline(file, line)) 
    {
        content += line + "\n";
    }
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

int main() {
    std::string key;
    std::cout << "Introduceti cheia: ";
    std::cin >> key;

    std::string text = readFile("input.txt");

    VigenereCipher cipher(key);

    //criptare si scriere in fisier
    std::string encrypted = cipher.encrypt(text);
    writeFile("encrypted.txt", encrypted);

    //decriptare si scriere in fisier
    std::string decrypted = cipher.decrypt(encrypted);
    writeFile("decrypted.txt", decrypted);


    return 0;
}