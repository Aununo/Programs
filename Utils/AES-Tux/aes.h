#ifndef AES_H
#define AES_H

#include <vector>

namespace aes {

const int N_ROUNDS = 10;
std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& key);
std::vector<unsigned char> decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key);

} // namespace aes

#endif // AES_H