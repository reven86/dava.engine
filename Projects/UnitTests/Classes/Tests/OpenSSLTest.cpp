#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>

DAVA_TESTCLASS (OpenSSLTest)
{
    char* password = "Hello Dava Engine!";
    String privateKeyFilePath;
    String publicKeyFilePath;

    OpenSSLTest()
    {
        String privateKeyFilePath = FilePath("~doc:/OpensslTest/PrivateKey.key").GetAbsolutePathname();
        String publicKeyFilePath = FilePath("~doc:/OpensslTest/PublicKey.key").GetAbsolutePathname();
    }

    DAVA_TEST (GenerateKeysTest)
    {
        FileSystem* fs = FileSystem::Instance();
        fs->DeleteDirectory("~doc:/OpensslTest/", true);
        fs->CreateDirectory("~doc:/OpensslTest/");

        FILE* privateKey = fopen(privateKeyFilePath.c_str(), "wb");
        FILE* publicKey = fopen(publicKeyFilePath.c_str(), "wb");
        TEST_VERIFY(privateKey != nullptr);
        TEST_VERIFY(publicKey != nullptr);

        const unsigned keyBits = 1024;
        RSA* rsa = RSA_generate_key(keyBits, RSA_F4, nullptr, nullptr);
        const EVP_CIPHER* cipher = EVP_get_cipherbyname("bf-ofb");

        int res = PEM_write_RSAPrivateKey(privateKey, rsa, cipher, nullptr, 0, nullptr, password);
        TEST_VERIFY(res == 1);

        res = PEM_write_RSAPublicKey(publicKey, rsa);
    }
};
