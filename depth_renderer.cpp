#include "common.cpp"

int WIND_X = 2000;
int WIND_Y = 700;

int PICT_X = 0;
int PICT_Y = 0;

const int BLOCK_SIZE = 10;
const int MAX_BLOCK_SHIFT = 20;

using namespace sf;

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

void calculate_depths (Image& left, Image& right, Image& depth)
	{
	printf ("pict x = %i, pict y = %i\n", PICT_X, PICT_Y);
	
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

int main (int argc, char** argv)
	{
	if (argc < 2)
		{
		fprintf (stderr, "Give config filename.\n");
		assert (false);
		}
	
	RenderWindow App (sf::VideoMode (WIND_X, WIND_Y, 32), "Elijah stereo _demo.");
	
	FILE* settings = fopen (argv [1], "rt");
	if (!settings) return -1;
	
	while (true)
		{
		if (feof (settings)) break;
		
		char command [BUFFER_SIZE] = { };
		
		fscanf (settings, "%s", &command);
		
		if (command [0] == '-') continue;
		
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
	
		if (!temp.loadFromFile (command)) return -1;
	
		Vector2u sz = temp.getSize ();
	
		PICT_X = sz.x / 2;
		PICT_Y = sz.y;
	
		left.create   (sz.x / 2, sz.y, Color (30, 60, 120));
		right.create  (sz.x / 2, sz.y, Color (60, 120, 30));
		depth.create  (PICT_X, PICT_Y, Color (0, 0, 0));
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
	
		calculate_depths (left, right, depth);
		
		concatenate ("_depth_", command);
		if (!depth.saveToFile(FILENAME_BUF))
    		return -1;
		}
	
	fclose (settings);
	
	return 0;
	}

