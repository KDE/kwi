// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 tint;

layout(std140, binding = 0) uniform Uniforms {
    mat4 modelView;
    mat4 projection;
    vec4 sampleBounds;
    vec4 parameters;
} uniforms;

void main() {
    vec4 devicePosition = uniforms.modelView * vec4(position, 0.0, 1.0);

    if (uniforms.parameters.z > 0.5) {
        float dpr = uniforms.parameters.y;

        // Round positions to 1/64 pixel before snapping bitmaps.
        // Floating-point transforms must not round two corners at the same
        // half-pixel phase in opposite directions and stretch a glyph.
        vec2 subpixelPosition = floor(devicePosition.xy * dpr * 64.0 + 0.5) / 64.0;
        devicePosition.xy = floor(subpixelPosition + 0.5) / dpr;
    }

    gl_Position = uniforms.projection * devicePosition;
    uv = texcoord;
    tint = color * uniforms.parameters.x;
}
