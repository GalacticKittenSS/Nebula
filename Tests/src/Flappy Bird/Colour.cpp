#include "Colour.h"

void Colour::Update(Nebula::Timestep ts) {
    hue += hueSpeed * ts;
	
    if (hue > 360.0f)
		hue = hue - 360;

    float s = 1;
    float v = 1;

    float C = s * v;
    float X = (float)(C * (1 - abs(fmod(hue / 60.0, 2) - 1)));
    float m = v - C;
    float r, g, b;
    
    if (hue >= 0 && hue < 60) {
        r = C, g = X, b = 0;
    }
    else if (hue >= 60 && hue < 120) {
        r = X, g = C, b = 0;
    }
    else if (hue >= 120 && hue < 180) {
        r = 0, g = C, b = X;
    }
    else if (hue >= 180 && hue < 240) {
        r = 0, g = X, b = C;
    }
    else if (hue >= 240 && hue < 300) {
        r = X, g = 0, b = C;
    }
    else {
        r = C, g = 0, b = X;
    }

    red   = (r + m);
    green = (g + m);
    blue  = (b + m);
}