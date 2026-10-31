// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "Symbols_p.h"

#include <QBrush>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <array>
#include <cmath>

namespace GhosttyQt::Quick
{
namespace
{
// Unicode Box Drawing arm weights: up, down, left, right (2 bits each).
// Values: 0 absent, 1 light, 2 heavy, 3 double.
constexpr std::array<quint8, 128> arms = {
    0x50, 0xa0, 0x05, 0x0a, 0x50, 0xa0, 0x05, 0x0a, 0x50, 0xa0, 0x05, 0x0a, 0x44, 0x84, 0x48, 0x88, 0x14, 0x24, 0x18, 0x28, 0x41, 0x81, 0x42, 0x82, 0x11, 0x21,
    0x12, 0x22, 0x45, 0x85, 0x46, 0x49, 0x4a, 0x86, 0x89, 0x8a, 0x15, 0x25, 0x16, 0x19, 0x1a, 0x26, 0x29, 0x2a, 0x54, 0x64, 0x94, 0xa4, 0x58, 0x68, 0x98, 0xa8,
    0x51, 0x61, 0x91, 0xa1, 0x52, 0x62, 0x92, 0xa2, 0x55, 0x65, 0x95, 0xa5, 0x56, 0x59, 0x5a, 0x66, 0x96, 0x69, 0x99, 0xa6, 0xa9, 0x6a, 0x9a, 0xaa, 0x50, 0xa0,
    0x05, 0x0a, 0xf0, 0x0f, 0xc4, 0x4c, 0xcc, 0x34, 0x1c, 0x3c, 0xc1, 0x43, 0xc3, 0x31, 0x13, 0x33, 0xc5, 0x4f, 0xcf, 0x35, 0x1f, 0x3f, 0xf4, 0x5c, 0xfc, 0xf1,
    0x53, 0xf3, 0xf5, 0x5f, 0xff, 0x44, 0x14, 0x11, 0x41, 0x00, 0x00, 0x00, 0x10, 0x01, 0x40, 0x04, 0x20, 0x02, 0x80, 0x08, 0x90, 0x09, 0x60, 0x06,
};

void box(QPainter &p, char32_t cp, QSize size, int thin)
{
    const int w{size.width()};
    const int h{size.height()};
    const int cx{(w - thin) / 2};
    const int cy{(h - thin) / 2};
    const int bits{arms[cp - 0x2500]};

    if (cp >= 0x256d && cp <= 0x2570) {
        p.setRenderHint(QPainter::Antialiasing);
        const qreal x{cx + thin / 2.0};
        const qreal y{cy + thin / 2.0};
        const bool right{(bits & 0xc0) != 0};
        const bool down{(bits & 0x0c) != 0};
        const qreal horizontal{right ? w - x : x};
        const qreal vertical{down ? h - y : y};
        const qreal radius{qMin(horizontal, vertical)};
        p.translate(x, y);
        p.scale(right ? 1 : -1, down ? 1 : -1);
        const qreal halfStroke{thin / 2.0};
        QPainterPath path;
        path.moveTo(horizontal, -halfStroke);
        path.lineTo(radius, -halfStroke);
        path.arcTo(QRectF(-halfStroke, -halfStroke, 2 * radius + thin, 2 * radius + thin), 90, 90);
        path.lineTo(-halfStroke, vertical);
        path.lineTo(halfStroke, vertical);
        path.lineTo(halfStroke, radius);
        path.arcTo(QRectF(halfStroke, halfStroke, 2 * radius - thin, 2 * radius - thin), 180, -90);
        path.lineTo(horizontal, halfStroke);
        path.closeSubpath();
        p.fillPath(path, Qt::white);

        return;
    }

    if (cp >= 0x2571 && cp <= 0x2573) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, thin, Qt::SolidLine, Qt::FlatCap));

        if (cp != 0x2572) {
            p.drawLine(QPointF(w, 0), QPointF(0, h));
        }

        if (cp != 0x2571) {
            p.drawLine(QPointF(0, 0), QPointF(w, h));
        }

        return;
    }

    for (int direction{0}; direction < 4; ++direction) {
        const int weight{(bits >> (direction * 2)) & 3};

        if (!weight) {
            continue;
        }

        const int thickness{weight == 2 ? thin * 2 : thin};
        const bool vertical{direction < 2};
        const bool negative{(direction & 1) == 0};
        const int extent{vertical ? h : w};
        const int center{vertical ? cy : cx};
        const int opposite{(bits >> ((direction ^ 1) * 2)) & 3};
        const int perpendicular{vertical ? bits >> 4 : bits & 15};
        const std::array<int, 2> neighbors{perpendicular & 3, (perpendicular >> 2) & 3};
        const int joinWidth{qMax(1, qMax(neighbors[0], neighbors[1])) * thin};
        const int farJoin{(extent - joinWidth) / 2 + (negative ? joinWidth : 0)};

        for (int track{0}; track < (weight == 3 ? 2 : 1); ++track) {
            int end{opposite ? center + (negative ? thin : 0) : farJoin};

            if ((weight == 3 && neighbors[track] == 3) || (weight != 3 && !opposite && neighbors[0] == 3 && neighbors[1] == 3)) {
                end = center + (negative ? 0 : thin);
            }

            const int across{((vertical ? w : h) - thickness) / 2 + (weight == 3 ? (track * 2 - 1) * thin : 0)};
            const int start{negative ? 0 : end};
            const int length{negative ? end : extent - end};
            p.fillRect(vertical ? QRect(across, start, thickness, length) : QRect(start, across, length, thickness), Qt::white);
        }
    }

    int dashes{0};
    bool vertical{false};

    if (cp >= 0x2504 && cp <= 0x250b) {
        dashes = cp <= 0x2507 ? 3 : 4;
        vertical = (cp & 2) != 0;
    } else if (cp >= 0x254c && cp <= 0x254f) {
        dashes = 2;
        vertical = (cp & 2) != 0;
    }

    if (dashes) {
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        const int extent{vertical ? h : w};

        for (int i{0}; i < dashes - 1; ++i) {
            const int start{qRound((i + 0.7) * extent / dashes)};
            const int end{qRound((i + 1.0) * extent / dashes)};
            p.fillRect(vertical ? QRect(0, start, w, end - start) : QRect(start, 0, end - start, h), Qt::transparent);
        }
    }
}

void branch(QPainter &p, char32_t cp, QSize size, int thin)
{
    const qreal x{(size.width() - thin) / 2 + thin / 2.0};
    const qreal y{(size.height() - thin) / 2 + thin / 2.0};
    const QPointF center(x, y);
    const std::array<QPointF, 4> edges{QPointF(x, 0), QPointF(x, size.height()), QPointF(0, y), QPointF(size.width(), y)};
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::white, thin, Qt::SolidLine, Qt::FlatCap));

    if (cp >= 0xf5ee) {
        // Filled/hollow pairs and edge connections from the character chart in
        // kitty PRs #7681 and #7805.
        constexpr std::array<int, 16> connections{0, 4, 8, 12, 2, 1, 3, 10, 6, 9, 5, 11, 7, 14, 13, 15};
        const int mask{connections[(cp - 0xf5ee) / 2]};
        const qreal radius{qMax(qreal(thin), size.width() * 0.32)};

        for (int arm{0}; arm < 4; ++arm) {
            if (mask & (1 << arm)) {
                const QPointF delta{edges[arm] - center};
                const qreal length{std::hypot(delta.x(), delta.y())};
                p.drawLine(center + delta * (radius / length), edges[arm]);
            }
        }

        p.setBrush((cp & 1) ? Qt::NoBrush : QBrush(Qt::white));
        p.drawEllipse(center, radius, radius);

        return;
    }

    if (cp <= 0xf5d1) {
        p.drawLine(cp == 0xf5d0 ? edges[2] : edges[0], cp == 0xf5d0 ? edges[3] : edges[1]);

        return;
    }

    if (cp <= 0xf5d5) {
        const int direction{cp == 0xf5d2 ? 3 : cp == 0xf5d3 ? 2 : cp == 0xf5d4 ? 1 : 0};
        const int opposite{direction ^ 1};
        QLinearGradient gradient(edges[opposite], edges[direction]);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(0.35, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
        p.setPen(QPen(QBrush(gradient), thin, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(edges[opposite], edges[direction]);

        return;
    }

    // Four curve bits: down-right, down-left, up-left, up-right.
    constexpr std::array<quint8, 24> curves{1, 2, 4, 8, 8, 1, 9, 4, 2, 6, 2, 1, 3, 4, 8, 12, 12, 3, 8, 4, 1, 2, 8, 4};
    constexpr std::array<quint8, 24> straight{0, 0, 0, 0, 3, 3, 0, 3, 3, 0, 12, 12, 0, 12, 12, 0, 3, 3, 7, 11, 11, 7, 14, 14};
    const auto index = cp - 0xf5d6;

    for (int arm{0}; arm < 4; ++arm) {
        if (straight[index] & (1 << arm)) {
            p.drawLine(center, edges[arm]);
        }
    }

    constexpr std::array<int, 4> vertical{1, 1, 0, 0};
    constexpr std::array<int, 4> horizontal{3, 2, 2, 3};

    for (int curve{0}; curve < 4; ++curve) {
        if (curves[index] & (1 << curve)) {
            QPainterPath path(edges[vertical[curve]]);
            path.quadTo(center, edges[horizontal[curve]]);
            p.drawPath(path);
        }
    }
}

void legacy(QPainter &p, char32_t cp, QSize size, int thin)
{
    const qreal w{qreal(size.width())};
    const qreal h{qreal(size.height())};

    if (cp <= 0x1fb67) {
        struct Cut {
            int x0, y0, x1, y1;
            bool right;
        };

        constexpr std::array<Cut, 22> cuts{{{0, 2, 1, 3, false}, {0, 2, 2, 3, false}, {0, 1, 1, 3, false}, {0, 1, 2, 3, false}, {0, 0, 1, 3, false},
                                            {0, 1, 1, 0, true},  {0, 1, 2, 0, true},  {0, 2, 1, 0, true},  {0, 2, 2, 0, true},  {0, 3, 1, 0, true},
                                            {0, 2, 2, 1, true},  {1, 3, 2, 2, true},  {0, 3, 2, 2, true},  {1, 3, 2, 1, true},  {0, 3, 2, 1, true},
                                            {1, 3, 2, 0, true},  {1, 0, 2, 1, false}, {0, 0, 2, 1, false}, {1, 0, 2, 2, false}, {0, 0, 2, 2, false},
                                            {1, 0, 2, 3, false}, {0, 1, 2, 2, false}}};
        const bool inverse{cp >= 0x1fb52};
        const auto cut = cuts[(cp - 0x1fb3c) % 22];
        const QPointF a(cut.x0 * w / 2, cut.y0 * h / 3);
        const QPointF b(cut.x1 * w / 2, cut.y1 * h / 3);
        const auto side = [a, b](QPointF point) {
            return (b.x() - a.x()) * (point.y() - a.y()) - (b.y() - a.y()) * (point.x() - a.x());
        };

        qreal sign{side(QPointF(cut.right ? w : 0, h)) < 0 ? -1.0 : 1.0};

        if (inverse) {
            sign = -sign;
        }

        const QPolygonF rectangle{QPointF(0, 0), QPointF(w, 0), QPointF(w, h), QPointF(0, h)};
        QPolygonF polygon;
        polygon.reserve(rectangle.size() + 1);

        for (int i{0}; i < rectangle.size(); ++i) {
            const QPointF current{rectangle[i]};
            const QPointF next{rectangle[(i + 1) % rectangle.size()]};
            const qreal s0{side(current) * sign};
            const qreal s1{side(next) * sign};

            if (s0 >= 0) {
                polygon += current;
            }

            if ((s0 < 0) != (s1 < 0)) {
                polygon += current + (next - current) * (s0 / (s0 - s1));
            }
        }

        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawPolygon(polygon);
    } else if (cp <= 0x1fb6f) {
        const auto edge = (cp - 0x1fb68) % 4;
        const std::array<QPointF, 4> corners{QPointF(0, h), QPointF(0, 0), QPointF(w, 0), QPointF(w, h)};
        QPainterPath triangle;
        triangle.moveTo(w / 2, h / 2);
        triangle.lineTo(corners[edge]);
        triangle.lineTo(corners[(edge + 1) % 4]);
        triangle.closeSubpath();
        QPainterPath fill;

        if (cp < 0x1fb6c) {
            fill.addRect(QRectF(0, 0, w, h));
        }

        fill.addPath(triangle);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillPath(fill, Qt::white);
    } else if (cp <= 0x1fb7b) {
        const bool horizontal{cp >= 0x1fb76};
        const auto stripe = (cp - (horizontal ? 0x1fb76 : 0x1fb70)) + 1;

        if (horizontal) {
            const int y{qRound(stripe * h / 8)};
            p.fillRect(0, y, w, qRound((stripe + 1) * h / 8) - y, Qt::white);
        } else {
            const int x{qRound(stripe * w / 8)};
            p.fillRect(x, 0, qRound((stripe + 1) * w / 8) - x, h, Qt::white);
        }
    } else if (cp <= 0x1fb81) {
        constexpr std::array<quint8, 6> edges{6, 5, 9, 10, 3, 0};
        const int mask{edges[cp - 0x1fb7c]};
        const int sw{qRound(w / 8)};
        const int sh{qRound(h / 8)};
        const int right{qRound(7 * w / 8)};
        const int bottom{qRound(7 * h / 8)};

        if (mask & 1)
            p.fillRect(0, 0, w, sh, Qt::white);

        if (mask & 2)
            p.fillRect(0, bottom, w, h - bottom, Qt::white);

        if (mask & 4)
            p.fillRect(0, 0, sw, h, Qt::white);

        if (mask & 8)
            p.fillRect(right, 0, w - right, h, Qt::white);

        if (cp == 0x1fb81) {
            for (const int stripe : {0, 2, 4, 7}) {
                const int y{qRound(stripe * h / 8)};
                p.fillRect(0, y, w, qRound((stripe + 1) * h / 8) - y, Qt::white);
            }
        }
    } else if (cp <= 0x1fb8b) {
        constexpr std::array<int, 5> portions{2, 3, 5, 6, 7};
        const int portion{portions[(cp - 0x1fb82) % 5]};

        if (cp <= 0x1fb86) {
            p.fillRect(0, 0, w, qRound(portion * h / 8), Qt::white);
        } else {
            const int x{qRound((8 - portion) * w / 8)};
            p.fillRect(x, 0, w - x, h, Qt::white);
        }
    } else if (cp <= 0x1fb94) {
        const int mx{qRound(w / 2)};
        const int my{qRound(h / 2)};
        const std::array<QRectF, 4> halves{QRectF(0, 0, mx, h), QRectF(mx, 0, w - mx, h), QRectF(0, 0, w, my), QRectF(0, my, w, h - my)};

        if (cp <= 0x1fb8f) {
            p.fillRect(halves[cp - 0x1fb8c], QColor(255, 255, 255, 128));
        } else {
            p.fillRect(QRectF(0, 0, w, h), QColor(255, 255, 255, 127));

            if (cp != 0x1fb90) {
                const int half{cp == 0x1fb91 ? 2 : cp == 0x1fb92 ? 3 : 1};
                p.fillRect(halves[half], Qt::white);
            }
        }
    } else if (cp <= 0x1fb96) {
        const int pitch{2 * thin};

        for (int y{0}; y < h; y += pitch) {
            for (int x{0}; x < w; x += pitch) {
                if (((x / pitch + y / pitch) & 1) == (cp == 0x1fb96)) {
                    p.fillRect(x, y, pitch, pitch, Qt::white);
                }
            }
        }
    } else if (cp == 0x1fb97) {
        for (int y{0}; y < h; y += 4 * thin) {
            p.fillRect(0, y, w, 2 * thin, Qt::white);
        }
    } else if (cp <= 0x1fb99) {
        p.setRenderHint(QPainter::Antialiasing);

        if (cp == 0x1fb99) {
            p.translate(w, 0);
            p.scale(-1, 1);
        }

        const int repeats{qMax(1, size.width() / (2 * thin))};
        const qreal spacing{w / repeats};
        const qreal halfWidth{thin * std::hypot(1.0, w / h) / 2};
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);

        for (int i{-repeats}; i <= repeats; ++i) {
            const qreal x{(i + 0.5) * spacing};
            path.moveTo(x - w - halfWidth, -h);
            path.lineTo(x - w + halfWidth, -h);
            path.lineTo(x + 2 * w + halfWidth, 2 * h);
            path.lineTo(x + 2 * w - halfWidth, 2 * h);
            path.closeSubpath();
        }

        p.fillPath(path, Qt::white);
    } else {
        p.setRenderHint(QPainter::Antialiasing);
        const std::array<QPointF, 4> corners{QPointF(0, 0), QPointF(w, 0), QPointF(w, h), QPointF(0, h)};
        QPainterPath path;

        if (cp <= 0x1fb9b) {
            for (int edge{cp == 0x1fb9a ? 0 : 1}; edge < 4; edge += 2) {
                path.moveTo(w / 2, h / 2);
                path.lineTo(corners[edge]);
                path.lineTo(corners[(edge + 1) % 4]);
                path.closeSubpath();
            }

            p.fillPath(path, Qt::white);
        } else {
            const auto corner = cp - 0x1fb9c;
            path.moveTo(corners[corner]);
            path.lineTo(corners[(corner + 1) % 4]);
            path.lineTo(corners[(corner + 3) % 4]);
            path.closeSubpath();
            p.fillPath(path, QColor(255, 255, 255, 128));
        }
    }
}
}

bool isProceduralSymbol(char32_t cp)
{
    return (cp >= 0x2500 && cp <= 0x259f) || (cp >= 0x2800 && cp <= 0x28ff) || (cp >= 0xe0b0 && cp <= 0xe0bf) || (cp >= 0xf5d0 && cp <= 0xf60d)
        || (cp >= 0x1fb00 && cp <= 0x1fb9f && cp != 0x1fb93);
}

QImage symbolImage(char32_t cp, QSize size, int strokeWidth)
{
    // Supersample the four rounded corners for accurate coverage at small sizes.
    const int scale{cp >= 0x256d && cp <= 0x2570 ? 4 : 1};
    QImage image(size * scale, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter p{&image};
    p.scale(scale, scale);
    const int w{size.width()};
    const int h{size.height()};
    const int thin{qMax(1, strokeWidth)};

    if (cp <= 0x257f) {
        box(p, cp, size, thin);
    } else if (cp <= 0x259f) {
        if (cp == 0x2580) {
            p.fillRect(0, 0, w, (h + 1) / 2, Qt::white);
        } else if (cp <= 0x2588) {
            const int y{qRound(h * (8 - (cp - 0x2580)) / 8.0)};
            p.fillRect(0, y, w, h - y, Qt::white);
        } else if (cp <= 0x258f) {
            p.fillRect(0, 0, qRound(w * (0x2590 - cp) / 8.0), h, Qt::white);
        } else if (cp == 0x2590) {
            const int x{qRound(w / 2.0)};
            p.fillRect(x, 0, w - x, h, Qt::white);
        } else if (cp <= 0x2593) {
            // Avoid some moire when transformed.
            p.fillRect(image.rect(), QColor(255, 255, 255, 64 * (cp - 0x2590)));
        } else if (cp == 0x2594) {
            p.fillRect(0, 0, w, qRound(h / 8.0), Qt::white);
        } else if (cp == 0x2595) {
            const int x{qRound(7 * w / 8.0)};
            p.fillRect(x, 0, w - x, h, Qt::white);
        } else {
            constexpr std::array<int, 10> quadrants{4, 8, 1, 13, 9, 7, 11, 2, 6, 14};
            const int mask{quadrants[cp - 0x2596]};
            const int mx{qRound(w / 2.0)};
            const int my{qRound(h / 2.0)};

            for (int q{0}; q < 4; ++q) {
                if (mask & (1 << q)) {
                    const int x0{(q & 1) ? mx : 0};
                    const int y0{(q & 2) ? my : 0};
                    p.fillRect(x0, y0, (q & 1) ? w - x0 : mx, (q & 2) ? h - y0 : my, Qt::white);
                }
            }
        }
    } else if (cp >= 0x2800 && cp <= 0x28ff) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        constexpr std::array<int, 8> column{0, 0, 0, 1, 1, 1, 0, 1};
        constexpr std::array<int, 8> row{0, 1, 2, 0, 1, 2, 3, 3};
        const qreal radius{qMax(qreal(0.5), qMin(w / 6.0, h / 10.0))};

        for (int bit{0}; bit < 8; ++bit) {
            if ((cp - 0x2800) & (1 << bit)) {
                p.drawEllipse(QPointF((column[bit] + 0.5) * w / 2, (row[bit] + 0.5) * h / 4), radius, radius);
            }
        }
    } else if (cp >= 0x1fb00 && cp <= 0x1fb3b) {
        auto mask = cp - 0x1fb00 + 1;

        if (mask >= 21) {
            ++mask;
        }

        if (mask >= 42) {
            ++mask;
        }

        for (int bit{0}; bit < 6; ++bit) {
            if (mask & (1 << bit)) {
                const int x0{qRound((bit % 2) * w / 2.0)};
                const int x1{qRound((bit % 2 + 1) * w / 2.0)};
                const int y0{qRound((bit / 2) * h / 3.0)};
                const int y1{qRound((bit / 2 + 1) * h / 3.0)};
                p.fillRect(x0, y0, x1 - x0, y1 - y0, Qt::white);
            }
        }
    } else if (cp >= 0x1fb3c) {
        legacy(p, cp, size, thin);
    } else if (cp >= 0xf5d0 && cp <= 0xf60d) {
        branch(p, cp, size, thin);
    } else {
        p.setRenderHint(QPainter::Antialiasing);
        const bool left{(cp & 2) != 0};
        const bool outline{(cp & 1) != 0};
        QPainterPath path;

        if (cp >= 0xe0b4 && cp <= 0xe0b7) {
            const QRectF ellipse(left ? 0 : -w, 0, 2 * w, h);
            path.addEllipse(ellipse);
        } else if (cp >= 0xe0b8) {
            if (outline) {
                const bool rising{left != ((cp & 4) != 0)};
                path.moveTo(rising ? w : 0, 0);
                path.lineTo(rising ? 0 : w, h);
            } else {
                path.moveTo(left ? w : 0, 0);
                path.lineTo(left ? 0 : w, (cp & 4) ? 0 : h);
                path.lineTo(left ? w : 0, h);
                path.closeSubpath();
            }
        } else {
            path.moveTo(left ? w : 0, 0);
            path.lineTo(left ? 0 : w, h / 2.0);
            path.lineTo(left ? w : 0, h);

            if (!outline) {
                path.closeSubpath();
            }
        }

        p.setPen(outline ? QPen(Qt::white, thin, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin) : QPen(Qt::NoPen));
        p.setBrush(outline ? Qt::NoBrush : QBrush(Qt::white));
        p.drawPath(path);
    }

    if (scale > 1) {
        p.end();

        return image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    return image;
}
}
