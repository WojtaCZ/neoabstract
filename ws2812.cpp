#include "ws2812.hpp"

ws2812::strip::strip(PIO pio, uint8_t statemachine, uint8_t gpio, uint16_t pixelCount, bool isRGBW)  : pio(pio), sm(statemachine), gpio(gpio), pcount(pixelCount), rgbw(isRGBW) {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, gpio, 800000, rgbw);
    for(uint16_t i = 0; i < pixelCount; i++){
        pixels.push_back(pixel(color(0,0,0)));
    }
}

void ws2812::strip::update() {
    for(pixel & pix : pixels){
        pio_sm_put_blocking(pio, sm, pix.getColor().raw() << 8u);
    }
}

void ws2812::strip::setColor(color col) {
    for(pixel & pix : pixels){
        pix.setColor(col);
    }
}


ws2812::substrip::substrip(strip & s, uint16_t from, uint16_t to) : str(s), from(from), to(to){
    for(uint16_t i = 0; i < s.getPixels().size(); i++){
        if(i >= from && i <= to){
            pixelMask.push_back(true);
        }else pixelMask.push_back(false);
    }
}


void ws2812::substrip::setColor(color col){
    uint16_t idx = 0;
    for(bool hasMask : pixelMask){
        if(hasMask) str.getPixels().at(idx).setColor(col);
        idx++;
    }
}


void ws2812::substrip::shiftHue(int16_t degrees){
    uint16_t idx = 0;
    for(bool hasMask : pixelMask){
        if(hasMask) str.getPixels().at(idx).shiftHue(degrees);
        idx++;
    }
}


void ws2812::color::rgb2hsv(uint8_t r, uint8_t g, uint8_t b, double & h, double & s, double & v){
    double min, max, delta, _r, _g, _b, _h, _s, _v;

    _r = (double)r/255.0;
    _g = (double)g/255.0;
    _b = (double)b/255.0;

    min = _r < _g ? _r : _g;
    min = min  < _b ? min  : _b;

    max = _r > _g ? _r : _g;
    max = max  > _b ? max  : _b;

    v = max;                                
    delta = max - min;

    if(delta < 0.00001){
        s = 0;
        h = 0; // undefined, maybe nan?
        return;
    }

    if( max > 0.0 ){ // NOTE: if Max is == 0, this divide would cause a crash
        s = (delta / max);                  // s
    }else{
        return;  
    }

    if(_r >= max){                           // > is bogus, just keeps compilor happy
        h = ( _g - _b ) / delta;        // between yellow & magenta
    }else{
        if(_g >= max){
            h = 2.0 + ( _b - _r ) / delta;  // between cyan & yellow
        }else h = 4.0 + ( _r - _g ) / delta;  // between magenta & cyan
    }
    
    h *= 60.0;                              // degrees

    if(h < 0.0) h += 360.0;
}


void ws2812::color::hsv2rgb(double h, double s, double v, uint8_t & r, uint8_t & g, uint8_t & b){
    double      hh, p, q, t, ff, _r, _g, _b;
    long        i;

    if(s <= 0.0) {       // < is bogus, just shuts up warnings
        _r = v;
        _g = v;
        _b = v;

        r = (uint8_t)(_r*255.0);
        g = (uint8_t)(_g*255.0);
        b = (uint8_t)(_b*255.0);
        return;
    }
    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch(i) {
    case 0:
        _r = v;
        _g = t;
        _b = p;
        break;
    case 1:
        _r = q;
        _g = v;
        _b = p;
        break;
    case 2:
        _r = p;
        _g = v;
        _b = t;
        break;

    case 3:
        _r = p;
        _g = q;
        _b = v;
        break;
    case 4:
        _r = t;
        _g = p;
        _b = v;
        break;
    case 5:
    default:
        _r = v;
        _g = p;
        _b = q;
        break;
    }   

    r = (uint8_t)(_r*255.0);
    g = (uint8_t)(_g*255.0);
    b = (uint8_t)(_b*255.0);
}

void ws2812::color::shiftHue(int16_t degrees){
    double h = 0, s = 0, v = 0;
    rgb2hsv(r, g, b, h, s, v);
    h += (double)(degrees % 360);
    hsv2rgb(h, s, v, r, g, b);
}

void ws2812::strip::shiftHue(int16_t degrees){
    for(pixel & pix : pixels){
        pix.shiftHue(degrees);
    }
}