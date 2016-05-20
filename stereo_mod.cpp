# include <SFML/Graphics.hpp>
#include "assert.h"
#include "math.h"

int PICT_X = 0;//505; //505;//1002; //960; //280;
int PICT_Y = 0;//621; //621;//900;  //658; //210;

const int SPACE = 10; //Space between pictures.

int WIND_X = PICT_X * 2 + 3 * SPACE;
int WIND_Y = PICT_Y + 2 * SPACE;

const int BLOCK_SIZE = 10;
const int MAX_BLOCK_SHIFT = 20;

const double ANGLE_STEP = 0.02;

using namespace sf;

#define PLACE_PICTURE( NAME, COLUMN, LINE )					  \
	output_texture.loadFromImage (NAME, rs); 				  \
	sprite.setPosition (sf::Vector2f (SPACE * (COLUMN + 1) + PICT_X * COLUMN, \
					  SPACE * (LINE   + 1) + PICT_Y * LINE)); \
	sprite.setTexture  (output_texture); 					  \
	App.draw(sprite); 

void render_to_image (Image& source, Image& target, Image& depth, double ang, bool right, Image& filled)
	{
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			int x = 0;
			
			if (!right) x = i + ang       * (depth.getPixel (i, j).r - 128);
			else	    x = i + (ang - 1) * (depth.getPixel (i, j).r - 128);
			
			if (x > PICT_X || x < 0) continue;
			
			target.setPixel (x, j, source.getPixel (i, j));
			filled.setPixel (x, j, Color (1, 1, 1));
			}
	
	}

int bound (int inp, int min, int max)
	{
	if (inp < min) return min;
	if (inp > max) return max;
	
	return inp;
	}

void calc_res (Image& left, Image& right, Image& result, Image& depth, double ang, Image& tl,
	       Image& tr, Image& filled1, Image& filled2)
	{
	render_to_image (right, tr, depth, ang, true,  filled2);
	render_to_image (left,  tl, depth, ang, false, filled1);
	
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			if (!filled1.getPixel (i, j).r)
				{
				if (!filled2.getPixel (i, j).r)
					{
					Color col (0, 0, 0);
					
					for (int k = -1; k < 2; k ++)
						for (int l = -1; l < 2; l ++)
							{
							Color temp = tl.getPixel (bound (i + k, 0, PICT_X),
										  bound (j + l, 0, PICT_Y));
							col.r += temp.r;
							col.g += temp.g;
							col.b += temp.b;
							}
					
					col.r /= 9;
					col.g /= 9;
					col.b /= 9;
					
					result.setPixel (i, j, col);
					}
				
				else result.setPixel (i, j, tr.getPixel (i, j));
				}
			
			else result.setPixel (i, j, tr.getPixel (i, j));
			}
	
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			Color temp = result.getPixel (i, j);
			
			if (temp.r > 250 && temp.g > 250 && temp.b > 250)
				{
				//result.setPixel (i, j, Color (255, 0, 0));
				
				int r = 0;
				int g = 0;
				int b = 0;
					
				for (int k = -1; k < 2; k ++)
					for (int l = -1; l < 2; l ++)
						{
						Color tempo = tl.getPixel (bound (i + k, 0, PICT_X),
									  bound (j + l, 0, PICT_Y));
						r += tempo.r;
						g += tempo.g;
						b += tempo.b;
						}
				
				r /= 9;
				g /= 9;
				b /= 9;
				
				result.setPixel (i, j, Color (r, g, b));
				
				}
			}
	}
	

void calculate_depths (Image& left, Image& right, Image& depth)
	{
	for (int i = 1; i < PICT_X - 1; i ++)
		{
		for (int j = 1; j < PICT_Y - 1; j ++)
			{
			int shift = - MAX_BLOCK_SHIFT;
			double cur_avg_min_sum = 1000000;
			
			for (int k = - MAX_BLOCK_SHIFT; k < MAX_BLOCK_SHIFT; k ++)
				{
				double avg_sum = 0;
				
				int dropped = 0;
				
				for (int l = - BLOCK_SIZE / 2; l < BLOCK_SIZE / 2; l ++)
					for (int m = - BLOCK_SIZE / 2; m < BLOCK_SIZE / 2; m ++)
						{
						if (i + l < 0 || j + m < 0 || i + k + l < 0 ||
						    i + l > WIND_X || j + m > WIND_Y || i + k + l > WIND_X)
							{
							dropped ++;
							continue;
							}
						
						Color le = left.getPixel (i + l, j + m);
						Color ri = right.getPixel (i + k + l, j + m);
						avg_sum += abs (le.r - ri.r) + abs (le.g - ri.g) + abs (le.b - ri.b);
						}
				
				avg_sum *= pow (BLOCK_SIZE, 2) / (pow (BLOCK_SIZE, 2) - dropped);
				
				if (avg_sum < cur_avg_min_sum)
					{
					shift = k;
					cur_avg_min_sum = avg_sum;
					}
				}
			
			depth.setPixel (i, j, Color (128 + shift, 128 + shift, 128 + shift));
			}
		}
	}

void smooth_depth (Image& depth)
	{
	for (int i = 3; i < PICT_X - 3; i ++)
		for (int j = 3; j < PICT_Y - 3; j ++)
			{
			int sum = 0;
			
			for (int k = -1; k < 2; k ++)
				for (int l = -1; l < 2; l ++)
					{
					sum += depth.getPixel (i + k, j + l).r;
					}
			
			depth.setPixel (i, j, Color (sum / 9, sum / 9, sum / 9));
			}
	}

void sub_mid_depth (Image& depth)
	{
	int min =  10000;
	int max = -10000;
	
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			Color col = depth.getPixel (i, j);
			
			if (col.r > max) max = col.r;
			if (col.r < min) min = col.r;
			}
	
	int mid = (max + min) / 3;
	
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			Color col = depth.getPixel (i, j);
			int newcol = col.r - mid;
			depth.setPixel (i, j, Color (newcol, newcol, newcol));
			}
	}

int main (int argc, char** argv)
	{
	if (argc < 2)
		{
		fprintf (stderr, "Give stereopare .png filename.\n");
		assert (false);
		}
	
	Image temp;
	Image left;
	Image right;
	Image depth;
	Image temp_res_l;
	Image temp_res_r;
	Image _filled1;
	Image _filled2;
	Image result;
	Texture output_texture;
	
	if (!temp.loadFromFile  (argv [1])) return -1;
	
	Vector2u sz = temp.getSize ();
	
	PICT_X = sz.x / 2;
	PICT_Y = sz.y;
	
	WIND_X = PICT_X * 2 + 3 * SPACE; //2 * PICT_X + 3 * SPACE;
	WIND_Y = PICT_Y + 2 * SPACE; //2 * PICT_Y + 3 * SPACE;
	
	RenderWindow App (sf::VideoMode (WIND_X, WIND_Y, 32), "Elijah stereo _demo.");
	
	left.create   (sz.x / 2, sz.y, Color (30, 60, 120));
	right.create  (sz.x / 2, sz.y, Color (60, 120, 30));
	depth.create  (PICT_X, PICT_Y, Color (200, 100, 100));
	result.create (PICT_X, PICT_Y, Color (100, 200, 100));
	temp_res_l.create (PICT_X, PICT_Y, Color (100, 200, 100));
	temp_res_r.create (PICT_X, PICT_Y, Color (100, 200, 100));
	_filled1.create (PICT_X, PICT_Y, Color (0, 0, 0));
	_filled2.create (PICT_X, PICT_Y, Color (0, 0, 0));
	
	for (int i = 0; i < sz.x / 2; i ++)
		for (int j = 0; j < sz.y; j ++)
			{
			left.setPixel  (i, j, temp.getPixel (i, j));
			right.setPixel (i, j, temp.getPixel (i + sz.x / 2, j));
			}
	
	double ang        = -ANGLE_STEP;
	bool   to_refresh = true;
	
	calculate_depths (left, right, depth);
	smooth_depth (depth);
	//sub_mid_depth (depth);
	
	while (App.isOpen())
		{
        	sf::Event Event;
       		while (App.pollEvent(Event))
			{
			if (Event.type == sf::Event::Closed) App.close();
        
			if (Event.type == sf::Event::KeyPressed)
				{
				switch (Event.key.code)
                			{
                			case sf::Keyboard::Escape: { App.close(); break; }
					case sf::Keyboard::Right:
						{
						if (ang + ANGLE_STEP <= 1 + ANGLE_STEP)
							{
							ang += ANGLE_STEP;
							to_refresh = true;
							}
						break;
						}
						
					case sf::Keyboard::Left:
						{
						if (ang - ANGLE_STEP >= -ANGLE_STEP)
							{
							ang -= ANGLE_STEP;
							to_refresh = true;
							}
						break;
						}
					
					default: break;
					}
				}
			}
		
		if (to_refresh)
			{
			calc_res (left, right, result, depth, ang, temp_res_l, temp_res_r, _filled1, _filled2);
			to_refresh = false;
			}
		
		App.clear(sf::Color (0, 0, 255));	
		
		sf::IntRect pictrect  (0, 0, PICT_X, PICT_Y);
		const IntRect& rs = pictrect;
		sf::Sprite sprite;
		
		PLACE_PICTURE (depth,  1, 0)
		PLACE_PICTURE (result, 0, 0)
		
		App.display();
		}

	return 0;
	}

