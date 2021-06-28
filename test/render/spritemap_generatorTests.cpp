// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "render/spritemap_generator.h"

#include "jactorioTests.h"

namespace jactorio::render
{
    class SpritemapCreationTest : public testing::Test
    {
    protected:
        RendererSprites rendererSprites_;
        data::PrototypeManager proto_;
    };

    TEST_F(SpritemapCreationTest, CreateSpritemap) {
        // Sprite data delete by guard
        proto_.Make<proto::Sprite>(
            "sprite1", proto::Sprite("test/graphics/test/test_tile.png", {proto::Sprite::SpriteGroup::terrain}));
        proto_.Make<proto::Sprite>(
            "sprite2", proto::Sprite("test/graphics/test/test_tile1.png", {proto::Sprite::SpriteGroup::terrain}));

        proto_.Make<proto::Sprite>(
            "sprite3", proto::Sprite("test/graphics/test/test_tile2.png", {proto::Sprite::SpriteGroup::gui}));
        proto_.Make<proto::Sprite>(
            "sprite4", proto::Sprite("test/graphics/test/test_tile3.png", {proto::Sprite::SpriteGroup::gui}));

        // Should filter out to only 2 entries
        const auto data = RendererSprites::CreateSpritemap(proto_, proto::Sprite::SpriteGroup::terrain, false);

        EXPECT_EQ(data.width, 68);  // 64 + 2(2)
        EXPECT_EQ(data.height, 34); // 32 + 2
    }

    TEST_F(SpritemapCreationTest, CreateSpritemapCategoryNone) {
        // If a sprite does not have a group specified (sprite_group::none):

        // Sprite data delete by guard
        // Terrain
        proto_.Make<proto::Sprite>(
            "sprite1", proto::Sprite("test/graphics/test/test_tile.png", {proto::Sprite::SpriteGroup::terrain}));
        proto_.Make<proto::Sprite>(
            "sprite2", proto::Sprite("test/graphics/test/test_tile1.png", {proto::Sprite::SpriteGroup::terrain}));

        // Gui
        proto_.Make<proto::Sprite>(
            "sprite3", proto::Sprite("test/graphics/test/test_tile2.png", {proto::Sprite::SpriteGroup::gui}));
        proto_.Make<proto::Sprite>(
            "sprite4", proto::Sprite("test/graphics/test/test_tile3.png", {proto::Sprite::SpriteGroup::gui}));

        // None
        proto_.Make<proto::Sprite>("spriteNone", proto::Sprite("test/graphics/test/test_tile.png", {}));

        // Should filter out to 3 entries, total width of 32 * 3
        const auto data = RendererSprites::CreateSpritemap(proto_, proto::Sprite::SpriteGroup::terrain, false);

        EXPECT_EQ(data.width, 102); // 96 + 3(2)
        EXPECT_EQ(data.height, 34); // 32 + 2
    }


    // ======================================================================

    /// For testing the actual generation algorithm
    class SpritemapGeneratorTest : public testing::Test
    {
    protected:
        using ColorT     = unsigned short;
        using ImageCoord = unsigned int;

        using RgbaPair = std::tuple<ColorT, ColorT, ColorT, ColorT>;
        using ImgPos   = std::tuple<ImageCoord, ImageCoord>;

        RendererSprites rendererSprites_;

        /// \return true if pixel contains specified color
        static void ValidatePixel(const RendererSprites::SpritemapData& spritemap,
                                  const ImgPos& xy,
                                  const RgbaPair& rgba) {
            const unsigned int offset = (spritemap.width * std::get<1>(xy) + std::get<0>(xy)) * 4;

            const auto* img_ptr = spritemap.spriteBuffer.get();
            EXPECT_EQ(img_ptr[offset + 0], std::get<0>(rgba));
            EXPECT_EQ(img_ptr[offset + 1], std::get<1>(rgba));
            EXPECT_EQ(img_ptr[offset + 2], std::get<2>(rgba));
            EXPECT_EQ(img_ptr[offset + 3], std::get<3>(rgba));
        }

        void TearDown() override {
            for (auto& prototype : prototypes_) {
                delete prototype;
            }
        }

        std::vector<proto::Sprite*> prototypes_;

        void AddSprite(const std::string& image_path) {
            auto* sprite       = new proto::Sprite;
            sprite->internalId = static_cast<unsigned>(nextId_);
            sprite->Load(image_path);

            prototypes_.push_back(sprite);
            nextId_++;
        }

    private:
        uint64_t nextId_ = 0;
    };

    TEST_F(SpritemapGeneratorTest, CreateBorderHorizontalStacked) {
        AddSprite("test/graphics/test/test_tile.png");
        AddSprite("test/graphics/test/test_tile.png");

        const auto spritemap = RendererSprites::GenSpritemap(prototypes_, false);

        EXPECT_EQ(spritemap.width, 68);
        EXPECT_EQ(spritemap.height, 34);

        // (Second sprite) Top, Right, Bottom, Left
        ValidatePixel(spritemap, {34 + 32, 0}, {14, 94, 255, 255});
        ValidatePixel(spritemap, {34 + 33, 1}, {14, 94, 255, 255});
        ValidatePixel(spritemap, {34 + 1, 33}, {27, 235, 59, 255});
        ValidatePixel(spritemap, {34 + 0, 2}, {238, 33, 255, 255});
    }

    TEST_F(SpritemapGeneratorTest, CreateBorderVerticalStacked) {
        AddSprite("test/graphics/test/30x60.png"); // A
        AddSprite("test/graphics/test/20x20.png"); // B
        AddSprite("test/graphics/test/21x20.png"); // C

        // Sprite stacking:
        // A C
        // A B

        const auto spritemap = RendererSprites::GenSpritemap(prototypes_, false);

        EXPECT_EQ(spritemap.width, 55);
        EXPECT_EQ(spritemap.height, 62);

        // (Sprite B) Top, Right, Bottom, Left
        ValidatePixel(spritemap, {32 + 1, 22 + 0}, {186, 69, 163, 255});
        ValidatePixel(spritemap, {32 + 21, 22 + 2}, {0, 128, 192, 255});
        ValidatePixel(spritemap, {32 + 1, 22 + 21}, {7, 248, 248, 255});
        ValidatePixel(spritemap, {32 + 0, 22 + 2}, {205, 151, 50, 255});
    }

    TEST_F(SpritemapGeneratorTest, GenSpritemapInverted) {
        // Images 0 - 2 are 32 x 32 px
        // Image 3 is 64 x 64

        AddSprite("test/graphics/test/test_tile.png");
        AddSprite("test/graphics/test/test_tile1.png");
        AddSprite("test/graphics/test/test_tile2.png");
        AddSprite("test/graphics/test/test_tile3.png");

        const auto spritemap = RendererSprites::GenSpritemap(prototypes_, true);

        EXPECT_EQ(spritemap.width, 168); // 160 + 2(4)
        EXPECT_EQ(spritemap.height, 66); // 32 + 2
    }

    TEST_F(SpritemapGeneratorTest, GenSpritemap) {
        // Images 0 - 2 are 32 x 32 px

        AddSprite("test/graphics/test/20x30.png");
        AddSprite("test/graphics/test/21x20.png");
        AddSprite("test/graphics/test/20x20.png");
        AddSprite("test/graphics/test/5x1.png");

        AddSprite("test/graphics/test/30x60.png");
        AddSprite("test/graphics/test/20x59.png");
        AddSprite("test/graphics/test/40x30.png");

        const auto spritemap = RendererSprites::GenSpritemap(prototypes_, false);

        EXPECT_EQ(spritemap.width, 118);
        EXPECT_EQ(spritemap.height, 62);
    }

    TEST_F(SpritemapGeneratorTest, GenEmptySpritemap) {
        const auto spritemap = RendererSprites::GenSpritemap(prototypes_, false);

        EXPECT_EQ(spritemap.width, 0);
        EXPECT_EQ(spritemap.height, 0);
    }
} // namespace jactorio::render
