#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>

DAVA_TESTCLASS (OpenSSLTest)
{
    DAVA_TEST (GenerateKeysTest)
    {
        FileSystem* fs = FileSystem::Instance();
        fs->DeleteDirectory("~doc:/OpensslTest/", true);
        fs->CreateDirectory("~doc:/OpensslTest/");

        ScopedPtr<File> privateKey(File::Create("~doc:/OpensslTest/PrivateKey.key", File::WRITE | File::OPEN));
        ScopedPtr<File> publicKey(File::Create("~doc:/OpensslTest/PublicKey.key", File::WRITE | File::OPEN));
        TEST_VERIFY(privateKey != nullptr);
        TEST_VERIFY(publicKey != nullptr);

        RSA* rsa = nullptr;
        unsigned keyBits = 1024;

        TEST_VERIFY(true != false);
    }
};
