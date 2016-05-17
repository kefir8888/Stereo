# include <SFML/Graphics.hpp>

const int PICT_X = 280;
const int PICT_Y = 210;

const int SPACE = 10; //Space between pictures.

const int WIND_X = 2 * PICT_X + 3 * SPACE;
const int WIND_Y = 2 * PICT_Y + 3 * SPACE;

using namespace sf;

#define PLACE_PICTURE( NAME, COLUMN, LINE )					  \
	output_texture.loadFromImage (NAME, rs); 				  \
	sprite.setPosition (sf::Vector2f (SPACE * (COLUMN + 1) + PICT_X * COLUMN, \
					  SPACE * (LINE   + 1) + PICT_Y * LINE)); \
	sprite.setTexture  (output_texture); 					  \
	App.draw(sprite); 

void calc_res () { }

int main()
	{
	RenderWindow App (sf::VideoMode (WIND_X, WIND_Y, 32), "Elijah stereo _demo.");
	Image left;
	Image right;
	Image depth;
	Image result;
	Texture output_texture;
	
	//const Image& im = pict;
	
	if (!left.loadFromFile  ("left.png"))  return -1;
	if (!right.loadFromFile ("right.png")) return -1;
	depth.create (PICT_X, PICT_Y, Color (100, 100, 100));
	
	double ang        = 0.1;
	bool   to_refresh = true;
	
	//calculate depths
	
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
					case sf::Keyboard::Right:  { ang += 0.1; to_refresh = true; break; }
					case sf::Keyboard::Left:   { ang -= 0.1; to_refresh = true; break; }
					default: break;
					}
				}
			}
		
		if (to_refresh)
			{
			calc_res ();
			to_refresh = false;
			}
		
		App.clear(sf::Color(0,0,255));	
		
		sf::IntRect pictrect  (0, 0, PICT_X, PICT_Y);
		const IntRect& rs = pictrect;
		sf::Sprite sprite;
		
		PLACE_PICTURE (left,   0, 0)
		PLACE_PICTURE (right,  1, 0)
		PLACE_PICTURE (depth,  0, 1)
		PLACE_PICTURE (result, 1, 1)
		
		App.display();
		
		//for (int i = 0; i < coord / 2; i ++)
		//	{
		//	pict.setPixel (coord, i, Color (100, 100, 100));
		//	}
		}

	return 0;
	}

