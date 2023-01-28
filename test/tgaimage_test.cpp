#include <gtest/gtest.h>

#include "tgaimage.h"

TEST(tgaimage_test, width) {
	TGAImage image(100, 100, TGAImage::RGB);
	ASSERT_TRUE(image.width() == 100) << "width is not 100";
}

int main() {
	TEST();
}