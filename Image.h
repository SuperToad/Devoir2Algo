//#include <jpeglib.h>
#include <iostream>
#include <stdexcept>
using namespace std;

typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned char ubyte;

class GrayImage
{
	private:
		ushort width;
		ushort height;
		ubyte* array;
	public:
		GrayImage();
		~GrayImage();
		GrayImage(ushort width, ushort height);
		GrayImage(const GrayImage &source);
		void clear(ubyte gris);
		inline ubyte& pixel(ushort x, ushort y) const;
		inline ubyte& pixel(ushort x, ushort y) { return array[y*width + x]; }
		inline const ubyte& operator() (ushort x, ushort y) const;
		inline ubyte& operator() (ushort x, ushort y);
		void rectangle(ushort x, ushort y, ushort w, ushort h, ubyte color);
		void fillRectangle(ushort x, ushort y, ushort w, ushort h, ubyte color);
		void writePGM(ostream& f);
		static GrayImage* readPGM(istream& f);
		
		inline ushort getWidth() {return width;}
		inline ushort getHeight() {return height;}
};

class Color
{
	public:
		Color(ubyte _r=0, ubyte _g=0, ubyte _b=0) : r(_r), g(_g), b(_b) {};
		// J ai du mettre les valeurs de r g et b en public pour y acceder plus tard
		ubyte r, g, b;
		inline ~Color() {}
		// Operations utilisees dans les fonctions de ColorImage
		inline friend Color operator+(const Color& c, const Color& c2) { return Color(ushort(c.r + c2.r), ushort(c.g + c2.g), ushort(c.b + c2.b)); }
		inline friend Color operator*(const double& d, const Color& c) { return Color(uint(double(c.r) * d), uint(double(c.g) * d), uint(double(c.b) * d)); }
};

class ColorImage
{
	
	private:
		ushort width;
		ushort height;
		Color* array;
	public:
		ColorImage();
		~ColorImage();
		ColorImage(ushort width, ushort height);
		ColorImage(const ColorImage &source);
		void clear(Color color);
		inline Color& pixel(ushort x, ushort y) const;
		inline Color& pixel(ushort x, ushort y);
		inline const Color& operator() (ushort x, ushort y) const;
		inline Color& operator() (ushort x, ushort y);
		void rectangle(ushort x, ushort y, ushort w, ushort h, Color color);
		void fillRectangle(ushort x, ushort y, ushort w, ushort h, Color color);
		void writePPM(ostream& f);
		ColorImage* readPPM(istream& f);
		ColorImage* simpleScale(ushort w, ushort h) const;
		ColorImage* bilinearScale(ushort w, ushort h) const;
		void writeTGA(ostream& f);
};

