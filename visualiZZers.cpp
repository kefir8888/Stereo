#include "common.cpp"

int PICT_X = 0;
int PICT_Y = 0;

const int SPACE = 10; //Space between pictures.

int WIND_X = 1000;
int WIND_Y = 700;

const double ANGLE_STEP = 0.02;

using namespace sf;

#define PLACE_PICTURE( NAME, COLUMN, LINE, PICX, PICY)				\
	output_texture.loadFromImage (NAME, rs); 				  \
	sprite.setPosition (sf::Vector2f (SPACE * (COLUMN + 1) + PICX * COLUMN, \
					  SPACE * (LINE   + 1) + PICY * LINE)); \
	sprite.setTexture  (output_texture); 					  \
	App.draw(sprite); 

int get_strings_count (char* filename)
	{
	FILE* inp = fopen (filename, "rt");
	//CHECK_STREAM (inp, filename)
	
	int res = 0;
	
	while (!feof (inp))
		{
		char* waste_buffer [BUFFER_SIZE] = { };
		fscanf (inp, "%s", &waste_buffer);
		res ++;
		}
	
	fclose (inp);
	
	return res - 1;
	}

struct Picture
	{
	Vector2u size;
	
	Image result;
	Image origin_depth;
	Image left;
	Image right;
	Image depth;
	Image temp_res_l;
	Image temp_res_r;
	Image _filled1;
	Image _filled2;
	};

void render_to_image (Image& source, Image& target, Image& depth, double ang, bool right, Image& filled, int szx, int szy)
	{
	//printf ("%i %i ", szx, szy);
	for (int i = 0; i < szx; i ++)
		for (int j = 0; j < szy; j ++)
			{
			int x = 0;
			
			if (!right) x = i + ang       * (depth.getPixel (i, j).r - 128);
			else	    x = i + (ang - 1) * (depth.getPixel (i, j).r - 128);
			
			if (x > szx || x < 0) continue;
			
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

//void calc_res (Image& left, Image& right, Image& result, Image& depth, double ang, Image& tl,
//	       Image& tr, Image& filled1, Image& filled2)
void calc_res (Picture& pict, double ang)
	{
	render_to_image (pict.right, pict.temp_res_r, pict.depth, ang, true,  pict._filled2, pict.size.x, pict.size.y);
	render_to_image (pict.left,  pict.temp_res_l, pict.depth, ang, false, pict._filled1, pict.size.x, pict.size.y);
	
	for (int i = 0; i < pict.size.x; i ++)
		for (int j = 0; j < pict.size.y; j ++)
			{
			if (!pict._filled1.getPixel (i, j).r)
				{
				if (!pict._filled2.getPixel (i, j).r)
					{
					Color col (0, 0, 0);
					
					for (int k = -1; k < 2; k ++)
						for (int l = -1; l < 2; l ++)
							{
							Color temp = pict.temp_res_l.getPixel
								(bound (i + k, 0, PICT_X), bound (j + l, 0, PICT_Y));
							col.r += temp.r;
							col.g += temp.g;
							col.b += temp.b;
							}
					
					col.r /= 9;
					col.g /= 9;
					col.b /= 9;
					
					pict.result.setPixel (i, j, col);
					//pict.result.setPixel (i, j, pict.temp_res_l.getPixel (i, j));
					}
				
				else pict.result.setPixel (i, j, pict.temp_res_r.getPixel (i, j));
				}
			else pict.result.setPixel (i, j, pict.temp_res_l.getPixel (i, j));
			}
	
	/*for (int i = 0; i < PICT_X; i ++)
		for (int j = 0; j < PICT_Y; j ++)
			{
			Color temp = pict.result.getPixel (i, j);
			
			if (temp.r > 250 && temp.g > 250 && temp.b > 250)
				{
				//result.setPixel (i, j, Color (255, 0, 0));
				
				int r = 0;
				int g = 0;
				int b = 0;
					
				for (int k = -1; k < 2; k ++)
					for (int l = -1; l < 2; l ++)
						{
						Color tempo = pict.temp_res_l.getPixel
							(bound (i + k, 0, PICT_X), bound (j + l, 0, PICT_Y));
						r += tempo.r;
						g += tempo.g;
						b += tempo.b;
						}
				
				r /= 9;
				g /= 9;
				b /= 9;
				
				pict.result.setPixel (i, j, Color (r, g, b));
				}
			}
	*/
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

void Init_picture (Picture& pict, char* pairfilename)
	{
	concatenate ("_depth_", pairfilename);
	
	if (!pict.origin_depth.loadFromFile (FILENAME_BUF))
		{
		printf ("Unable to open %s file!\n", FILENAME_BUF);
		return;
		}
	
	Image temp;
	
	if (!temp.loadFromFile (pairfilename))
		{
		printf ("Unable to open %s file!\n", pairfilename);
		return;
		}
	
	pict.size = temp.getSize ();
	
	pict.size.x /= 2;
	
	//fprintf (stderr, "fuck");
	
	pict.depth.create      (pict.size.x, pict.size.y, Color (200, 100, 100));
	pict.temp_res_l.create (pict.size.x, pict.size.y, Color (100, 200, 100));
	pict.left.create       (pict.size.x, pict.size.y, Color (100, 200, 100));
	pict.right.create      (pict.size.x, pict.size.y, Color (100, 200, 100));
	pict.temp_res_r.create (pict.size.x, pict.size.y, Color (100, 200, 100));
	pict._filled1.create   (pict.size.x, pict.size.y, Color (0, 0, 0));
	pict._filled2.create   (pict.size.x, pict.size.y, Color (0, 0, 0));
	pict.result.create     (pict.size.x, pict.size.y, Color (0, 0, 0));
	//pict.origin_depth.getPixel (5, 5);
	fprintf (stderr, "bldjad");
	
	for (int i = 0; i < pict.size.x; i ++)
		for (int j = 0; j < pict.size.y; j ++)
			{
			pict.left.setPixel  (i, j, temp.getPixel (i, j));
			//fprintf (stderr, "well");
			pict.right.setPixel (i, j, temp.getPixel (i + pict.size.x, j));
			//fprintf (stderr, "welll");
			pict.depth.setPixel  (i, j, pict.origin_depth.getPixel (i, j));
			//fprintf (stderr, "wellll");
			}
	}

int main (int argc, char** argv)
	{
	if (argc < 2)
		{
		fprintf (stderr, "Give settings filename.\n");
		assert (false);
		}
	
	Texture output_texture;
	
	RenderWindow App (sf::VideoMode (WIND_X, WIND_Y, 32), "Elijah stereo _demo.");
	
	double ang        = -ANGLE_STEP;
	bool   to_refresh = true;
	
	//calculate_depths (left, right, depth);
	//smooth_depth (depth);
	//sub_mid_depth (depth);
	
	FILE* settings = fopen (argv [1], "rt");
	if (!settings) return -1;
	
	std::vector <Picture> pictures;
	
	int filescount = get_strings_count (argv [1]);
	
	printf ("pict count %i\n", filescount);
	
	while (true)
		{
		filescount --;
		if (filescount <= -1) break;
		
		fprintf (stderr, "lalala");
		if (feof (settings)) break;
		
		char pict_name_buf [BUFFER_SIZE] = { };
		fscanf (settings, "%s", &pict_name_buf);
		
		if (pict_name_buf [0] == '-') continue;
		fprintf (stderr, "ffuu");
		Picture pict;
		Init_picture (pict, pict_name_buf);
		
		pictures.push_back (pict);
		}
	
	printf ("%ipictures\n", pictures.size ());
	
	fclose (settings);
	
	if (!pictures.size ())
		{
		fprintf (stderr, "No pictures to work on.\n");
		return 0;
		}
	
	int curr_pict = 0;
	
	while (App.isOpen())
		{
		//fprintf (stderr, "hui");
		
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
						if (ang - ANGLE_STEP >= -1 -ANGLE_STEP)
							{
							ang -= ANGLE_STEP;
							to_refresh = true;
							}
						break;
						}
					
					case sf::Keyboard::Down:
						{
						if (curr_pict + 1 >= pictures.size ()) curr_pict -= pictures.size ();
						curr_pict ++;
						to_refresh = true;
						
						break;
						}
					
					case sf::Keyboard::Up:
						{
						if (curr_pict - 1 < 0) curr_pict += pictures.size ();
						curr_pict --;
						to_refresh = true;
						
						break;
						}
					
					default: break;
					}
				}
			}
		
		//fprintf (stderr, "chlen");
		
		if (to_refresh)
			{
			printf ("curr pict %i\n", curr_pict);
			
			//calc_res (left, right, result, depth, ang, temp_res_l, temp_res_r, _filled1, _filled2);
			calc_res (pictures [curr_pict], ang);
			to_refresh = false;
			}
		
		App.clear(sf::Color (0, 0, 255));	
		
		//sf::IntRect pictrect  (WIND_X / 2 - pictures [curr_pict].size.x / 2,
		//		       WIND_Y / 2 - pictures [curr_pict].size.y / 2,
		//		       pictures [curr_pict].size.x, pictures [curr_pict].size.y);
		
		sf::IntRect pictrect  (0, 0, pictures [curr_pict].size.x, pictures [curr_pict].size.y);
		
		const IntRect& rs = pictrect;
		sf::Sprite sprite;
		
		//PLACE_PICTURE (depth,  1, 0)
		PLACE_PICTURE (pictures [curr_pict].result, 0, 0, pictures [curr_pict].size.x, pictures [curr_pict].size.y)
		
		App.display();
		
		/*
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
		*/
		}

	return 0;
	}

