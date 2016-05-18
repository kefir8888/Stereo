# include <SFML/Graphics.hpp>

const int PICT_X = 280;//505;//1002; //960; //280;
const int PICT_Y = 210;//621;//900;  //658; //210;

const int SPACE = 10; //Space between pictures.

const int WIND_X = PICT_X * 2 + 3 * SPACE; //2 * PICT_X + 3 * SPACE;
const int WIND_Y = PICT_Y + 2 * SPACE; //2 * PICT_Y + 3 * SPACE;

const int BLOCK_SIZE = 10;

const int MAX_BLOCK_SHIFT = 20;

using namespace sf;

#define PLACE_PICTURE( NAME, COLUMN, LINE )					  \
	output_texture.loadFromImage (NAME, rs); 				  \
	sprite.setPosition (sf::Vector2f (SPACE * (COLUMN + 1) + PICT_X * COLUMN, \
					  SPACE * (LINE   + 1) + PICT_Y * LINE)); \
	sprite.setTexture  (output_texture); 					  \
	App.draw(sprite); 

void calc_res (Image& left, Image& result, Image& depth, double ang)
	{
	for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			int x = i + ang * depth.getPixel (i, j).r;
			
			if (x > PICT_X || x < 0) continue;
			
			result.setPixel (x, j, left.getPixel (i, j));
			}
	}

void calculate_depths (Image& left, Image& right, Image& depth)
	{
	/*for (int i = 0; i < 20; i ++)
		for (int j = 0; j < 30; j ++)
			depth.setPixel (i, j, Color (100, 100, 100));
	*/
	
	for (int i = MAX_BLOCK_SHIFT + BLOCK_SIZE  /2; i < PICT_X - MAX_BLOCK_SHIFT - BLOCK_SIZE  /2; i ++)
		{
		for (int j = MAX_BLOCK_SHIFT + BLOCK_SIZE / 2; j < PICT_Y - MAX_BLOCK_SHIFT - BLOCK_SIZE / 2; j ++)
			{
			int shift = - MAX_BLOCK_SHIFT;
			int cur_minimal_sum = 1000000;
			
			for (int k = - MAX_BLOCK_SHIFT; k < MAX_BLOCK_SHIFT; k ++)
				{
				int cur_sum = 0;
				
				for (int l = - BLOCK_SIZE / 2; l < BLOCK_SIZE / 2; l ++)
					for (int m = - BLOCK_SIZE / 2; m < BLOCK_SIZE / 2; m ++)
						{
						Color le = left.getPixel (i + l, j + m);
						Color ri = right.getPixel (i + k + l, j + m);
						cur_sum += abs (le.r - ri.r) + abs (le.g - ri.g) + abs (le.b - ri.b);
						}
				
				if (cur_sum < cur_minimal_sum)
					{
					shift = k;
					cur_minimal_sum = cur_sum;
					}
				}
			
			depth.setPixel (i, j, Color (10 * shift, 10 * shift, 10 * shift));
			//depth.setPixel (i, j, Color (128 + 10 * shift, 128 + 10 * shift, 128 + 10 * shift));
			}
		}
	}

void smooth_depth (Image& depth)
	{
	for (int i = 3; i < WIND_X - 3; i ++)
		for (int j = 3; j < WIND_Y - 3; j ++)
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

int main()
	{
	RenderWindow App (sf::VideoMode (WIND_X, WIND_Y, 32), "Elijah stereo _demo.");
	Image left;
	Image right;
	Image depth;
	Image result;
	//Image& left
	Texture output_texture;
	
	//const Image& im = pict;
	
	if (!left.loadFromFile  ("left.png")) return -1;
	if (!right.loadFromFile ("right.png")) return -1;
	depth.create  (PICT_X, PICT_Y, Color (200, 100, 100));
	result.create (PICT_X, PICT_Y, Color (100, 200, 100));
	
	double ang        = 0.01;
	bool   to_refresh = true;
	
	calculate_depths (left, right, depth);
	smooth_depth (depth);
	
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
					case sf::Keyboard::Right:  { ang += 0.01; to_refresh = true; break; }
					case sf::Keyboard::Left:   { ang -= 0.01; to_refresh = true; break; }
					default: break;
					}
				}
			}
		
		if (to_refresh)
			{
			calc_res (left, result, depth, ang);
			to_refresh = false;
			}
		
		App.clear(sf::Color (0, 0, 255));	
		
		sf::IntRect pictrect  (0, 0, PICT_X, PICT_Y);
		const IntRect& rs = pictrect;
		sf::Sprite sprite;
		
		//PLACE_PICTURE (left,   0, 0)
		//PLACE_PICTURE (right,  1, 0)
		PLACE_PICTURE (depth,  1, 0)
		PLACE_PICTURE (result, 0, 0)
		
		App.display();
		
		
		//for (int i = 0; i < coord / 2; i ++)
		//	{
		//	pict.setPixel (coord, i, Color (100, 100, 100));
		//	}
		}

	return 0;
	}

