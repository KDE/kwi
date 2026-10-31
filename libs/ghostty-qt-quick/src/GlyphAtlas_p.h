// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QHash>
#include <QImage>
#include <QPointF>
#include <QRawFont>
#include <QRect>
#include <QSGTexture>
#include <QSize>
#include <cstddef>
#include <memory>
#include <vector>

class QQuickWindow;

namespace GhosttyQt::Quick
{
struct AtlasPage {
    QImage image;
    std::unique_ptr<QSGTexture> texture;
    int x{1};
    int y{1};
    int rowHeight{0};
    bool dirty{false};
    quint64 lastUsed{0};
};

struct GlyphImage {
    enum class Format {
        Grayscale,
        Subpixel,
        Color,
    };

    std::shared_ptr<AtlasPage> page;
    QRect pixels;
    QPointF bearing;
    Format format{Format::Grayscale};
};

class GlyphAtlas
{
public:
    static constexpr qsizetype PixelBudget{32 * 1024 * 1024};

    explicit GlyphAtlas(QQuickWindow *window, quint64 *rasterizedGlyphs = nullptr, std::unique_ptr<GlyphAtlas> source = {});

    [[nodiscard]] GlyphImage glyph(const QRawFont &font, quint32 glyphIndex, bool allowSubpixel = false);
    [[nodiscard]] GlyphImage symbol(char32_t codepoint, QSize cellSize, int strokeWidth);
    [[nodiscard]] GlyphImage underline(int style, QSize cellSize, int strokeWidth);

    void beginFrame();
    void flush(quint64 *uploadedBytes = nullptr);
    void trim();

    [[nodiscard]] int pageCount() const;
    [[nodiscard]] qsizetype byteSize() const;

private:
    enum class Kind {
        Font,
        Symbol,
        Underline,
    };

    struct Key {
        QRawFont font;
        quint32 index{0};
        Kind kind{Kind::Font};
        bool subpixel{false};

        bool operator==(const Key &) const = default;

        friend std::size_t qHash(const Key &key, std::size_t seed) noexcept
        {
            return qHashMulti(seed, key.font, key.index, key.kind, key.subpixel);
        }
    };

    struct CachedGlyph {
        std::weak_ptr<AtlasPage> page;
        QRect pixels;
        QPointF bearing;
        GlyphImage::Format format{GlyphImage::Format::Grayscale};
    };

    [[nodiscard]] GlyphImage find(const Key &key);
    [[nodiscard]] GlyphImage insert(const Key &key, const QImage &image, QPointF bearing, GlyphImage::Format format, bool rasterized = true);

    QQuickWindow *m_window;
    QHash<Key, CachedGlyph> m_glyphs;
    std::vector<std::shared_ptr<AtlasPage>> m_pages;
    quint64 m_frame{0};
    quint64 *m_rasterized;
    std::unique_ptr<GlyphAtlas> m_source;
};
}
