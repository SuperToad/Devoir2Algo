#include <stdio.h>
#include "Image.h"
#include <iostream>
#include <stdexcept>

using namespace std;

const char *identifier = "duffaut_j";
const char *informations = "Le TGA j ai pas vraiment compris...";

// skip_line(is) permet de sauter toute une série d'octets de "is" jusqu'à trouver un '\n'
static void skip_line(istream& is)
{
	char c;
	do {
		is.get(c);      // Lire un caractère
	} while(c!='\n'); // tant qu'on n'atteint pas la fin de ligne.
}

// skip_comments(is) utilise la fonction précédente pour sauter zéro, une ou plusieurs lignes
// de commentaires débutées par '#' et allant jusqu'à '\n'.
static void skip_comments(istream& is)
{
	char c;
	is.get(c);       // Lire un caractère.
	while(c=='#')    // Tant que c'est un '#'.
	{
		skip_line(is); // On élimine les caractères jusqu'à la fin de ligne,
		is.get(c);     // Et on lit un nouveau caractère.
	}
	is.putback(c);   // On remet le caractère lu puisqu'il n'est pas un '#'.
}

/* GrayImage methods */

GrayImage::GrayImage(ushort _width, ushort _height)
: width(_width), height(_height), array(NULL)
{
	array = new ubyte[width*height];
}

GrayImage::~GrayImage()
{
	delete [] array;
}

GrayImage::GrayImage()
{
	throw runtime_error("Ne pas utiliser le constructeur par defaut pour GrayImage");
}

GrayImage::GrayImage(const GrayImage &source)
: width(source.width), height(source.height), array(NULL)
{
	array = new ubyte[width*height];
	for (int i; i < (width*height); i++)
		array[i] = source.array[i];
}

void GrayImage::clear (ubyte gris)
{
	for (int i = 0; i < (width*height); i++)
		array[i] = gris;
}

inline ubyte& GrayImage::pixel(ushort x, ushort y) const
	{ return array[y*width + x]; }
inline ubyte& GrayImage::pixel(ushort x, ushort y)
	{ return array[y*width + x]; }

inline const ubyte& GrayImage::operator() (ushort x, ushort y) const
	{ return array[y*width + x]; }
inline ubyte& GrayImage::operator() (ushort x, ushort y)
	{ return array[y*width + x]; }
	
void GrayImage::rectangle(ushort x, ushort y, ushort w, ushort h, ubyte color)
{
	for (int i = x; i < x+w+1; i++)
	{
		pixel(i, y) = color;
		pixel(i, y+h) = color;
	}
	for (int i = y; i < y+h; i++)
	{
		pixel(x, i) = color;
		pixel(x+w, i) = color;
	}
}

void GrayImage::fillRectangle(ushort x, ushort y, ushort w, ushort h, ubyte color)
{
	for (int i = x; i < x + w; i++)
		for (int j = y; j < y + h; j++)
			pixel(i, j) = color;

}

void GrayImage::writePGM(ostream& f)
{
	f << "P5\n";
	f << "#TP1\n";
	f << width << ' ' << height << endl;
	f << "#Image\n";
	f << "255\n";
	f.write((const char*) array, width*height);
}

GrayImage* GrayImage::readPGM(istream& f)
{
	string line;
	getline(f, line);
	if (line == "P5")
	{
		skip_comments(f);
		int newWidth, newHeight;
		f >> newWidth;
		f >> newHeight;
		skip_line(f);
		skip_comments(f);
		getline(f, line);
		if (line == "255")
		{
			GrayImage * newGray = new GrayImage(newWidth, newHeight);
			ubyte * array = new ubyte[newWidth*newHeight];
			f.read((char*)array, newWidth*newHeight);
			for (int j = 0; j < newHeight; j++)
				for (int i = 0; i < newWidth; i++)
					newGray->pixel(i, j) = array[j*newWidth + i];
			delete [] array;
			return newGray;
		}
		else
			throw runtime_error("Pas 255, connais pas.");
	}
	else
		throw runtime_error("Le fichier n est pas au format PGM");
	return new GrayImage(100, 400);
}

/* ColorImage methods */

ColorImage::ColorImage(ushort _width, ushort _height)
: width(_width), height(_height), array(NULL)
{
	array = new Color[width*height];
}

ColorImage::~ColorImage()
{
	delete [] array;
}

ColorImage::ColorImage()
{
	throw runtime_error("Ne pas utiliser le constructeur par defaut pour ColorImage");
}

ColorImage::ColorImage(const ColorImage &source)
: width(source.width), height(source.height), array(NULL)
{
	array = new Color[width*height];
	for (int i; i < (width*height); i++)
		array[i] = source.array[i];
}

void ColorImage::clear (Color color)
{
	for (int i = 0; i < (width*height); i++)
		array[i] = color;
}

inline Color& ColorImage::pixel(ushort x, ushort y) const
	{ return array[y*width + x]; }
inline Color& ColorImage::pixel(ushort x, ushort y)
	{ return array[y*width + x]; }

inline const Color& ColorImage::operator() (ushort x, ushort y) const
	{ return array[y*width + x]; }
inline Color& ColorImage::operator() (ushort x, ushort y)
	{ return array[y*width + x]; }
	
void ColorImage::rectangle(ushort x, ushort y, ushort w, ushort h, Color color)
{
	for (int i = x; i < x+w+1; i++)
	{
		pixel(i, y) = color;
		pixel(i, y+h) = color;
	}
	for (int i = y; i < y+h; i++)
	{
		pixel(x, i) = color;
		pixel(x+w, i) = color;
	}
}

void ColorImage::fillRectangle(ushort x, ushort y, ushort w, ushort h, Color color)
{
	for (int i = y; i < y + w; i++)
		for (int j = x; j < x + h; j++)
			pixel(i, j) = color;

}

void ColorImage::writePPM(ostream& f)
{
	f << "P6\n";
	f << "#TP2\n";
	f << width << ' ' << height << endl;
	f << "#Image\n";
	f << "255\n";
	f.write((const char*) array, width*height*3 );
}

ColorImage* ColorImage::readPPM(istream& f)
{
	string line;
	getline(f, line);
	if (line == "P6")
	{
		skip_comments(f);
		int newWidth, newHeight;
		f >> newWidth;
		f >> newHeight;
		skip_line(f);
		skip_comments(f);
		getline(f, line);
		if (line == "255")
		{
			ColorImage * newColor = new ColorImage(newWidth, newHeight);
			Color * array = new Color[newWidth*newHeight];
			f.read((char*)array, newWidth*newHeight*3);
			for (int j = 0; j < newHeight; j++)
				for (int i = 0; i < newWidth; i++)
					newColor->pixel(i, j) = array[j*newWidth + i];
			
			delete [] array;
			return newColor;
		}
		else
			throw runtime_error("Le programme ne traite que pour du 255");
	}
	else
		throw runtime_error("Le fichier n est au format PPM");
	return new ColorImage(100, 400);
}

ColorImage* ColorImage::simpleScale(ushort w, ushort h) const
{
	ColorImage *ip = new ColorImage(w, h);
	for (ushort yp = 0; yp < ip->height; yp++)
		for (ushort xp = 0; xp < ip->width; xp++)
		{
			double x = (double(xp)/ip->width)*width;
			double y = (double(yp)/ip->height)*height;
			(*ip)(xp, yp) = pixel((ushort)x, (ushort)y);
		}
	return ip;
}

ColorImage* ColorImage::bilinearScale(ushort w, ushort h) const
{
	ColorImage *ip = new ColorImage(w, h);
	for (ushort yp = 0; yp < ip->height; yp++)
		for (ushort xp = 0; xp < ip->width; xp++)
		{
			double x = (double(xp)/ip->width)*width;
			double y = (double(yp)/ip->height)*height;
			ushort xi = ushort(x);
			ushort yi = ushort(y);
			ushort xi1 = (xi+1 >= width? xi : xi+1);
			ushort yi1 = (yi+1 >= height? yi : yi+1);
			double lamba = x - xi;
			double mu = y - yi;
			(*ip)(xp, yp) = (1 - lamba)*(1 - mu)*pixel(xi, yi) + lamba*(1 - mu)*pixel(xi1, yi)
				+ (1 - lamba)*mu*pixel(xi, yi1)  + lamba*mu*pixel(xi1, yi1);
		}
	
	return ip;
}
