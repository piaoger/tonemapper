/*
    src/uncharted.h -- Uncharted tonemapping operator

    Copyright (c) 2017 Piaoger Gong

    Tone Mapper is provided under the MIT License.
    See the LICENSE.txt file for the conditions of the license.
*/

#pragma once

#include <tonemap.h>

class ACESOperator : public TonemapOperator {
public:
    ACESOperator() : TonemapOperator() {

        const float A = 2.51f;
        const float B = 0.03f;
        const float C = 2.43f;
        const float D = 0.59f;
        const float E = 0.14f;

        parameters["Gamma"] = Parameter(2.2f, 0.f, 10.f, "gamma", "Gamma correction value");
        parameters["A"] = Parameter(2.51f, 0.f, 10.f, "A", "Shoulder strength curve parameter");
        parameters["B"] = Parameter(0.03f, 0.f, 1.f, "B", "Linear strength curve parameter");
        parameters["C"] = Parameter(2.43f, 0.f, 10.f, "C", "Linear angle curve parameter");
        parameters["D"] = Parameter(0.59f, 0.f, 1.f, "D", "Toe strength curve parameter");
        parameters["E"] = Parameter(0.14f, 0.f, 1.f, "E", "Toe numerator curve parameter");

        name = "ACES";
        description = "ACES\n\nBy John Hable from the \"Filmic Tonemapping for Real-time Rendering\" Siggraph 2010 Course by Haarm-Pieter Duiker.";

        shader->init(
            "ACES",

            "",

            ""
        );
    }

    void process(const Image *image, uint8_t *dst, float exposure, float *progress) const override {

        const nanogui::Vector2i &size = image->getSize();
        *progress = 0.f;
        float delta = 1.f / (size.x() * size.y());

        float gamma = parameters.at("Gamma").value;
        float A = parameters.at("A").value;
        float B = parameters.at("B").value;
        float C = parameters.at("C").value;
        float D = parameters.at("D").value;
        float E = parameters.at("E").value;

        for (int i = 0; i < size.y(); ++i) {
            for (int j = 0; j < size.x(); ++j) {
                const Color3f &color = image->ref(i, j);
                Color3f c = Color3f(map(color.r(), exposure, A, B, C, D, E),
                                    map(color.g(), exposure, A, B, C, D, E),
                                    map(color.b(), exposure, A, B, C, D, E));
                c = c.clampedValue();
                c = c.gammaCorrect(gamma);
                dst[0] = (uint8_t) (255.f * c.r());
                dst[1] = (uint8_t) (255.f * c.g());
                dst[2] = (uint8_t) (255.f * c.b());
                dst += 3;
                *progress += delta;
            }
        }

    }

    float graph(float value) const override {
        float gamma = parameters.at("Gamma").value;
        float A = parameters.at("A").value;
        float B = parameters.at("B").value;
        float C = parameters.at("C").value;
        float D = parameters.at("D").value;
        float E = parameters.at("E").value;

        value = map(value, 1.f, A, B, C, D, E);
        value = clamp(value, 0.f, 1.f);
        value = std::pow(value, 1.f / gamma);
        return value;
    }

protected:
    float map(float v, float exposure, float A, float B, float C, float D, float E) const {
        float value = exposure * v;
        float exposureBias = 3.6f;
        value = mapAux(exposureBias * value, A, B, C, D, E);
        return value;
    }

protected:
    float mapAux(float x, float A, float B, float C, float D, float E) const {
        return (x * (A * x + B)) / (x * (C * x + D) + E);
        //return ((x * (A*x + C*B) + D*E) / (x * (A*x+B) + D*F)) - E/F;
    }
};