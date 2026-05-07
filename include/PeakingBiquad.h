#pragma once
#include <Audio.h>
#include <cmath>

class AudioFilterPeaking : public AudioFilterBiquad {
  public:
    AudioFilterPeaking() : AudioFilterBiquad() {}

    void setPeaking(uint32_t stage, float fc, float fs, float Q, float gainDB) {
        double A = std::pow(10.0, gainDB / 40.0);
        double w0 = 2.0 * M_PI * fc / fs;
        double cosW0 = std::cos(w0);
        double sinW0 = std::sin(w0);

        double alpha = sinW0 / (2.0 * Q);
        double b0 = 1 + alpha * A;
        double b1 = -2 * cosW0;
        double b2 = 1 - alpha * A;
        double a0 = 1 + alpha / A;
        double a1 = -2 * cosW0;
        double a2 = 1 - alpha / A;

        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;

        const double scale = 1073741824.0;
        int coef[5];

        coef[0] = b0 * scale;
        coef[1] = b1 * scale;
        coef[2] = b2 * scale;
        coef[3] = a1 * scale;
        coef[4] = a2 * scale;

        setCoefficients(stage, coef);
    }
};
