// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;
// Offset within the cell and its size.
layout(location = 3) in vec4 cell;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 tint;

layout(std140, binding = 0) uniform Uniforms {
    mat4 modelView;
    mat4 projection;
    vec4 sampleBounds;
    vec4 parameters;
    vec2 atlasSize;
} uniforms;

vec2 pixelPosition(vec2 position) {
    return floor(floor(position * 64.0 + 0.5) / 64.0 + 0.5);
}

void main() {
    vec4 devicePosition = uniforms.modelView * vec4(position, 0.0, 1.0);
    uv = texcoord;

    if (uniforms.parameters.z > 0.5) {
        float dpr = uniforms.parameters.y;

        // Round positions to 1/64 pixel before snapping bitmaps.
        // Floating-point transforms must not round two corners at the same
        // half-pixel phase in opposite directions and stretch a glyph.
        vec2 pixel = pixelPosition(devicePosition.xy * dpr);

        if (cell.z > 0.0) {
            vec2 origin = (uniforms.modelView * vec4(position - cell.xy, 0.0, 1.0)).xy * dpr;
            vec2 first = pixelPosition(origin);
            vec2 extent = pixelPosition(origin + cell.zw * dpr) - first;
            vec2 minimum = max(floor(cell.zw * dpr), vec2(1.0));
            vec2 variant = clamp(extent - minimum, vec2(0.0), vec2(1.0));
            vec2 atlasSize = uniforms.atlasSize;
            vec2 source = texcoord * atlasSize - cell.xy / cell.zw * minimum;
            uv = (source + variant * (minimum + 2.0) + pixel - first) / atlasSize;
        }

        devicePosition.xy = pixel / dpr;
    }

    gl_Position = uniforms.projection * devicePosition;
    tint = color * uniforms.parameters.x;
}
