// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#version 440

layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 tint;

layout(location = 0) out vec4 fragment;

layout(binding = 1) uniform sampler2D atlas;

layout(std140, binding = 0) uniform Uniforms {
    mat4 modelView;
    mat4 projection;
    vec4 sampleBounds;
    vec4 parameters;
} uniforms;

void main() {
    vec4 coverage = texture(atlas, clamp(uv, uniforms.sampleBounds.xy, uniforms.sampleBounds.zw));

    if (abs(uniforms.parameters.w) > 0.5) {
        float alpha = dot(coverage.rgb, vec3(1.0 / 3.0));

        // Negative modes use grayscale coverage. Positive modes 1..3 replay
        // source-over for one RGB channel; mode 4 uses a constant foreground.
        float mask = uniforms.parameters.w < 0.0 ? alpha
            : uniforms.parameters.w < 1.5 ? coverage.r
            : uniforms.parameters.w < 2.5 ? coverage.g : coverage.b;
        fragment = uniforms.parameters.w > 3.5 ? vec4(coverage.rgb, alpha) * tint.a : tint * mask;
    } else {
        fragment = coverage * tint;
    }
}
