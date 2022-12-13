#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <span>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"


namespace ws2812{

    class color{
        private:
            uint8_t r, g, b;   
            float intensity;
            void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, double & h, double & s, double & v);
            void hsv2rgb(double h, double s, double v, uint8_t & r, uint8_t & g, uint8_t & b);
        public:
            color(uint8_t r, uint8_t g, uint8_t b, float intensity = 1) : r(r), g(g), b(b), intensity(intensity) { };
            uint32_t raw(){ return (((uint32_t)((float)r*intensity) << 8) | ((uint32_t)((float)g*intensity) << 16) | (uint32_t)((float)b*intensity)); };
            void shiftHue(int16_t degrees);
    };

    class pixel{
        private:
            color col;
        public:
            pixel(color c) : col(c){}
            color getColor(){ return col; };
            void setColor(color c){ col = c; };
            void shiftHue(int16_t degrees) {if(col.raw() != 0) col.shiftHue(degrees); };

    };

    class strip{
        private:
            PIO pio;
            uint8_t sm, gpio, pcount;
            bool rgbw;
            std::vector<pixel> pixels;
        public: 
            strip(PIO pio, uint8_t statemachine, uint8_t gpio, uint16_t pixelCount, bool isRGBW = false);
            ~strip(){};
            void update();
            void setColor(color col);
            std::vector<pixel> & getPixels() { return pixels; };
            void shiftHue(int16_t degrees);
    };

    class substrip {
        private: 
            std::vector<bool> pixelMask;
            strip & str;
            uint16_t from, to;
        public:
            substrip(strip & s, uint16_t from, uint16_t to);
            //substrip(strip & s, std::vector<uint16_t> pixelIndexes);
            void setColor(color col);
            auto getPixels() {
                std::span whole(str.getPixels());
                std::span part{whole.subspan(from, to - from + 1)};
                return part;
            }

            void shiftHue(int16_t degrees);

    };

}