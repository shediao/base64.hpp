#include <gtest/gtest.h>

#include <optional>
#include <span>
#include <string>
#include <vector>

#include "base64/base64.hpp"

TEST(Base64Test, Basic) {
    const std::string kText = "hello world";
    const std::string kBase64Text = "aGVsbG8gd29ybGQ=";

    std::string decoded;
    bool ok;

    std::string encoded = base64::encode(kText);
    EXPECT_EQ(kBase64Text, encoded);

    ok = base64::decode(encoded, &decoded);
    EXPECT_TRUE(ok);
    EXPECT_EQ(kText, decoded);
}

TEST(Base64Test, Binary) {
    const uint8_t kData[] = {0x00, 0x01, 0xFE, 0xFF};

    std::string binary_encoded =
        base64::encode(std::string_view((const char*)kData, std::size(kData)));

    // Check that encoding the same data through the StringPiece interface gives
    // the same results.
    std::string string_piece_encoded;
    base64::encode(
        std::string(reinterpret_cast<const char*>(kData), sizeof(kData)),
        &string_piece_encoded);

    EXPECT_EQ(binary_encoded, string_piece_encoded);

    auto decode = base64::decode(binary_encoded);
    EXPECT_TRUE(decode.has_value());
    EXPECT_EQ(decode.value().size(), std::size(kData));
    for (size_t i = 0; i < std::size(kData); i++) {
        EXPECT_EQ(kData[i], decode.value()[i]);
    }
    EXPECT_FALSE(base64::decode("invalid base64!"));
}

TEST(Base64Test, InPlace) {
    const std::string kText = "hello world";
    const std::string kBase64Text = "aGVsbG8gd29ybGQ=";

    std::string text = base64::encode(kText);
    EXPECT_EQ(kBase64Text, text);

    bool ok = base64::decode(text, &text);
    EXPECT_TRUE(ok);
    EXPECT_EQ(text, kText);
}
