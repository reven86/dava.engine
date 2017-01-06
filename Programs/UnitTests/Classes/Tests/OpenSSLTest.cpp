#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include <random>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

using namespace DAVA;

DAVA_TESTCLASS (OpenSSLTest)
{
    String password = "Hello Dava Engine!";
    Vector<double> data;
    Vector<unsigned char> encryptedData;

    OpenSSLTest()
    {
        FileSystem* fs = FileSystem::Instance();
        fs->DeleteDirectory("~doc:/OpensslTest/", true);
        fs->CreateDirectory("~doc:/OpensslTest/");

        std::random_device rd;
        std::mt19937 gen(rd());
        for (unsigned n = 0; n < 1024; ++n)
        {
            data.push_back(std::generate_canonical<double, 666>(gen));
        }
    }

    ~OpenSSLTest()
    {
        FileSystem* fs = FileSystem::Instance();
        fs->DeleteDirectory("~doc:/OpensslTest/", true);
    }

    String GetPrivateKeyFilePath()
    {
        return FilePath("~doc:/OpensslTest/PublicKey.key").GetAbsolutePathname();
    }

    String GetPublicKeyFilePath()
    {
        return FilePath("~doc:/OpensslTest/PrivateKey.key").GetAbsolutePathname();
    }

    DAVA_TEST (GenerateKeysTest)
    {
        FILE* privateKey = fopen(GetPrivateKeyFilePath().c_str(), "wb");
        FILE* publicKey = fopen(GetPublicKeyFilePath().c_str(), "wb");
        SCOPE_EXIT
        {
            if (privateKey != nullptr)
            {
                fclose(privateKey);
            }
            if (publicKey != nullptr)
            {
                fclose(publicKey);
            }
        };

        if (privateKey == nullptr || publicKey == nullptr)
        {
            TEST_VERIFY_WITH_MESSAGE(false, "Cannot create key files");
            return;
        }

        const unsigned keyBits = 1024;
        RSA* rsa = RSA_generate_key(keyBits, RSA_F4, nullptr, nullptr);
        const EVP_CIPHER* cipher = EVP_get_cipherbyname("bf-ofb");
        SCOPE_EXIT
        {
            RSA_free(rsa);
        };

        char* pwdData = const_cast<char*>(password.c_str());
        int res = PEM_write_RSAPrivateKey(privateKey, rsa, cipher, nullptr, 0, nullptr, pwdData);
        TEST_VERIFY(res == 1);

        res = PEM_write_RSAPublicKey(publicKey, rsa);
        TEST_VERIFY(res == 1);
    }

    DAVA_TEST (EncryptTest)
    {
        FILE* publicKeyFile = fopen(GetPublicKeyFilePath().c_str(), "rb");
        RSA* publicKey = PEM_read_RSAPublicKey(publicKeyFile, nullptr, nullptr, nullptr);
        SCOPE_EXIT
        {
            if (publicKey != nullptr)
            {
                RSA_free(publicKey);
            }
            if (publicKeyFile != nullptr)
            {
                fclose(publicKeyFile);
            }
        };

        if (publicKeyFile == nullptr || publicKey == nullptr)
        {
            TEST_VERIFY_WITH_MESSAGE(false, "Cannot open or read public key file");
            return;
        }

        OpenSSL_add_all_algorithms();
        int allDataLen = static_cast<int>(data.size() * sizeof(decltype(data)::value_type));
        const unsigned char* ptr = reinterpret_cast<const unsigned char*>(data.data());
        Vector<unsigned char> tmp(RSA_size(publicKey));

        while (allDataLen != 0)
        {
            // must be less than RSA_size(rsa) - 11
            int dataLen = std::min(static_cast<int>(RSA_size(publicKey)) - 11, allDataLen);
            int res = RSA_public_encrypt(dataLen, ptr, tmp.data(), publicKey, RSA_PKCS1_PADDING);
            TEST_VERIFY(res == RSA_size(publicKey));

            ptr += dataLen;
            allDataLen -= dataLen;
            encryptedData.insert(encryptedData.end(), tmp.begin(), tmp.begin() + res);
        }
    }

    DAVA_TEST (DecryptTest)
    {
        OpenSSL_add_all_algorithms();
        FILE* privateKeyFile = fopen(GetPrivateKeyFilePath().c_str(), "rb");
        char* pwdData = const_cast<char*>(password.c_str());
        RSA* privateKey = PEM_read_RSAPrivateKey(privateKeyFile, nullptr, nullptr, pwdData);
        SCOPE_EXIT
        {
            if (privateKey != nullptr)
            {
                RSA_free(privateKey);
            }
            if (privateKeyFile != nullptr)
            {
                fclose(privateKeyFile);
            }
        };

        if (privateKeyFile == nullptr || privateKey == nullptr)
        {
            TEST_VERIFY_WITH_MESSAGE(false, "Cannot open or read private key file");
            return;
        }

        int keySize = RSA_size(privateKey);
        Vector<unsigned char> decryptedData;
        Vector<unsigned char> tmp(keySize);
        const unsigned char* ptr = reinterpret_cast<const unsigned char*>(encryptedData.data());
        int allDataLen = static_cast<int>(encryptedData.size());
        TEST_VERIFY(allDataLen % keySize == 0);

        while (allDataLen != 0)
        {
            int dataLen = RSA_private_decrypt(keySize, ptr, tmp.data(), privateKey, RSA_PKCS1_PADDING);
            TEST_VERIFY(dataLen >= 0 && dataLen <= keySize);

            allDataLen -= keySize;
            ptr += keySize;
            decryptedData.insert(decryptedData.end(), tmp.begin(), tmp.begin() + dataLen);
        }

        size_t originalDataLen = data.size() * sizeof(decltype(data)::value_type);
        TEST_VERIFY(originalDataLen == decryptedData.size());
        TEST_VERIFY(memcmp(data.data(), decryptedData.data(), originalDataLen) == 0);
    }
};
