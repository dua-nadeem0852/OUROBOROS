#include <iostream>
#include <cctype>
#include <SFML/Graphics.hpp>
using namespace std;


void player_movements(char map[][10], int &moverow, int& movecol, char direction) // 10 * 10 matrix
{
	if (direction == 'R') //right
	{
		if (movecol < 9)
		{
			movecol++;
		}
	}
	else if (direction == 'L') //left
	{
		if (movecol > 0)
		{
			movecol--;
		}
	}
	else if (direction == 'U') //up
	{
		if (moverow > 0)   //going from down to up till row 0 so it decreases
		{
			moverow--;
		}
	}
	else if (direction == 'D') //down
	{
		if (moverow < 9)
		{
			moverow++;
		}
	}
}

void already_visited(char map[][10], bool visited[][10], int rowofplayer, int colofplayer)
{
	visited[rowofplayer][colofplayer] = true;
}

char get_tile(char map[][10], int rowofplayer, int colofplayer)
{
	char grid_tile = map[rowofplayer][colofplayer];
	return grid_tile;
}

void interaction_with_tile(char grid_tile)
{
	if (grid_tile == 'Y')
	{
		//hydra
	}
	else if (grid_tile == 'B')
	{
		//bat
	}
	else if (grid_tile == 'E')
	{
		//scientist
	}

	else if (grid_tile == 'W')
	{
		//wizard
	}

	else if (grid_tile == 'H')
	{
		//hole
	}
	else if (grid_tile == 'C')
	{
		//traesure chest
	}
	else if (grid_tile == 'K')
	{
		//key
	}
	else if (grid_tile == 'R')
	{
		//relic
	}
	else if (grid_tile == 'D')
	{
		//diamond
	}
	else if (grid_tile == 'r')
	{
		//key relic
	}
	else if (grid_tile == 'M')
	{
		//player map
	}
	else if (grid_tile == 'N')
	{
		//necklace
	}
	else if (grid_tile == 'P')
	{
		//paper
	}
	else if (grid_tile == 'h')
	{
		//bush
	}
	else if (grid_tile == 'H')
	{
		//hole
	}
	else if (grid_tile == 's')
	{
		//script
	}
	else if (grid_tile == 'S')
	{
		//spiral
	}
	else if (grid_tile == 'b')
	{
		//bag
	}
	else if (grid_tile == 'w')
	{
		//wall
	}
	else
	{
		// ground
	}
}

int main()
{
	//Y = hydra //B = Bat //E = scientist
	//H = hole //C = chest //K=key
	//R = relic  //D = diamond // r = key relic
	// M = playmap // N = necklace // P = paper 
	// h = bush  // W = wizard // s = scipt
	// S = spiral // b = bag // w = wall // G = ground

	char map[10][10] = {
		{ 's', '.' , 'w', 'G' , 'h' , 'R' , 'G' , 'N' , 'W' , '.'},
		{ '.', 'W' , '.', 'r' , '.' , 'B' , '.' , 'b' , 'G' , 'K'},
		{ 'G', '.' , 'Y', '.' , 'h' , '.' , '.' , '.' , '.' , 'Y'},
		{ '.', 'S' , '.', 'M' , '.' , 'M' , 'h' , '.' , 'r' , '.'},
		{ 'h', '.' , 'R', '.' , 'FA' ,'.' , '.' , '.' , '.' , 'H'},
		{ '.', 'G' , '.', 'D' , '.' , '.' , '.' , 'R' , '.' , '.'},
		{ 's', '.' , 'E', '.' , '.' , 'C' , 'H' , '.' , 'P' , 'w'},
		{ '.', 'G' , '.', 'P' , '.' , 'B' , '.' , 'W' , '.' , '.'},
		{ '.', '.' , 'H', '.' , 'S' , '.' , 'E' , '.' , 'D' , '.'},
		{ 'G', 'b' , '.', 'N' , '.' , 'h' , 'G' , 's' , '.' , 'h'},
	};

	int rowofplayer = 4;
	int colofplayer = 4;
	char direction;
	char grid_tile;

	bool visited[10][10] = { false };
	visited[4][4] = true;

	already_visited(map, visited, rowofplayer, colofplayer);


	sf::RenderWindow
		window(sf::VideoMode(900, 900), "OURBOROUS");
	sf::RectangleShape rectangle(sf::Vector2f(90, 90));
	rectangle.setFillColor(sf::Color(110, 130, 105, 210));

	rectangle.setOutlineThickness(1);
	rectangle.setOutlineColor(sf::Color::Black);

	sf::Texture treasure;                         // treasure
	treasure.loadFromFile("treasure.png");
	sf::Sprite treasure_sprite;
	treasure_sprite.setTexture(treasure);
	treasure_sprite.setScale(
		90.f /                          // f means floating point number, calculating scale factor 
		treasure.getSize().x,
		90.f /
		treasure.getSize().y
	);

	sf::Texture key;                              // key
	key.loadFromFile("key.png");
	sf::Sprite key_sprite;
	key_sprite.setTexture(key);
	key_sprite.setScale(
		90.f /
		key.getSize().x,
		90.f /
		key.getSize().y
	);

	sf::Texture hole;                           // hole
	hole.loadFromFile("hole.png");
	sf::Sprite hole_sprite;
	hole_sprite.setTexture(hole);
	hole_sprite.setScale(
		90.f /
		hole.getSize().x,
		90.f /
		hole.getSize().y
	);

	sf::Texture bush;                         // bush
	bush.loadFromFile("bush.png");
	sf::Sprite bush_sprite;
	bush_sprite.setTexture(bush);
	bush_sprite.setScale(
		90.f /
		bush.getSize().x,
		90.f /
		bush.getSize().y
	);

	sf::Texture scientist;                         // Scientist
	scientist.loadFromFile("scientist.png");
	sf::Sprite scientist_sprite;
	scientist_sprite.setTexture(scientist);
	scientist_sprite.setScale(
		90.f /
		scientist.getSize().x,
		90.f /
		scientist.getSize().y
	);

	sf::Texture bat;                         // Bat
	bat.loadFromFile("bat.png");
	sf::Sprite bat_sprite;
	bat_sprite.setTexture(bat);
	bat_sprite.setScale(
		90.f /
		bat.getSize().x,
		90.f /
		bat.getSize().y
	);

	sf::Texture hydra;                         // Hydra
	hydra.loadFromFile("hydra.png");
	sf::Sprite  hydra_sprite;
	hydra_sprite.setTexture(hydra);
	hydra_sprite.setScale(
		90.f /
		hydra.getSize().x,
		90.f /
		hydra.getSize().y
	);

	sf::Texture wizard;                         // wizard
	wizard.loadFromFile("wizard.png");
	sf::Sprite wizard_sprite;
	wizard_sprite.setTexture(wizard);
	wizard_sprite.setScale(
		90.f /
		wizard.getSize().x,
		90.f /
		wizard.getSize().y
	);

	sf::Texture diamond;                         // diamond
	diamond.loadFromFile("diamond.png");
	sf::Sprite diamond_sprite;
	diamond_sprite.setTexture(diamond);
	diamond_sprite.setScale(
		90.f /
		diamond.getSize().x,
		90.f /
		diamond.getSize().y
	);

	sf::Texture keyrelic;                         // key relic
	keyrelic.loadFromFile("keyrelic.png");
	sf::Sprite keyrelic_sprite;
	keyrelic_sprite.setTexture(keyrelic);
	keyrelic_sprite.setScale(
		90.f /
		keyrelic.getSize().x,
		90.f /
		keyrelic.getSize().y
	);

	sf::Texture playmap;                         // play map
	playmap.loadFromFile("playmap.png");
	sf::Sprite playmap_sprite;
	playmap_sprite.setTexture(playmap);
	playmap_sprite.setScale(
		90.f /
		playmap.getSize().x,
		90.f /
		playmap.getSize().y
	);

	sf::Texture necklace;                         // necklace
	necklace.loadFromFile("necklace.png");
	sf::Sprite necklace_sprite;
	necklace_sprite.setTexture(necklace);
	necklace_sprite.setScale(
		90.f /
		necklace.getSize().x,
		90.f /
		necklace.getSize().y
	);

	sf::Texture paper;                         // paper
	paper.loadFromFile("paper.png");
	sf::Sprite paper_sprite;
	paper_sprite.setTexture(paper);
	paper_sprite.setScale(
		90.f /
		paper.getSize().x,
		90.f /
		paper.getSize().y
	);

	sf::Texture relic;                         // relic
	relic.loadFromFile("relic.png");
	sf::Sprite relic_sprite;
	relic_sprite.setTexture(relic);
	relic_sprite.setScale(
		90.f /
		relic.getSize().x,
		90.f /
		relic.getSize().y
	);

	sf::Texture script;                         // script
	script.loadFromFile("script.png");
	sf::Sprite script_sprite;
	script_sprite.setTexture(script);
	script_sprite.setScale(
		90.f /
		script.getSize().x,
		90.f /
		script.getSize().y
	);

	sf::Texture spiral;                         // spiral
	spiral.loadFromFile("spiral.png");
	sf::Sprite spiral_sprite;
	spiral_sprite.setTexture(spiral);
	spiral_sprite.setScale(
		90.f /
		spiral.getSize().x,
		90.f /
		spiral.getSize().y
	);

	sf::Texture wall;                         // wall
	wall.loadFromFile("wall.png");
	sf::Sprite wall_sprite;
	wall_sprite.setTexture(wall);
	wall_sprite.setScale(
		90.f /
		wall.getSize().x,
		90.f /
		wall.getSize().y
	);

	sf::Texture bag;                         // bag
	bag.loadFromFile("bag.png");
	sf::Sprite bag_sprite;
	bag_sprite.setTexture(bag);
	bag_sprite.setScale(
		90.f /
		bag.getSize().x,
		90.f /
		bag.getSize().y
	);

	sf::Texture ground;                         // ground
	ground.loadFromFile("ground.png");
	sf::Sprite ground_sprite;
	ground_sprite.setTexture(ground);
	ground_sprite.setScale(
		90.f /
		ground.getSize().x,
		90.f /
		ground.getSize().y
	);


	sf::Texture mist;               //Mist 
	mist.loadFromFile("mist.png");
	sf::Sprite mist_sprite;
	mist_sprite.setTexture(mist);
	mist_sprite.setColor(sf::Color(180,180,180,250)); // last value handling the mist transparency

	mist_sprite.setScale( // * by the orginal image width & height by 90
		90.f /
		mist.getSize().x,  //width
		90.f /
		mist.getSize().y); //height


	sf::Texture character;             //character
	cout << character.loadFromFile("viktor.png");
	sf::Sprite character_sprite;
	character_sprite.setTexture(character);

	character_sprite.setScale(
		90.f /
		character.getSize().x,
		90.f /
		character.getSize().y
	);

	while (window.isOpen())
	{
		window.clear(); //clears the previous frame
		for (int i = 0; i < 10;i++)
		{
			for (int j = 0;j < 10;j++)
			{
				ground_sprite.setPosition(j * 90, i * 90);
				window.draw(ground_sprite);

				rectangle.setPosition(j * 90, i * 90);
				window.draw(ground_sprite);

				//if (i == rowofplayer && j == colofplayer)
				//{
				//	window.draw(ground_sprite);
				//}
				//window.draw(rectangle);
			}
		}

		for (int i = 0; i < 10;i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (map[i][j] == 'C')
				{
					treasure_sprite.setPosition(j * 90, i * 90);
					window.draw(treasure_sprite);
				}

				if (map[i][j] == 'K')
				{
					key_sprite.setPosition(j * 90, i * 90);
					window.draw(key_sprite);
				}

				if (map[i][j] == 'h')
				{
					bush_sprite.setPosition(j * 90, i * 90);
					window.draw(bush_sprite);
				}

				if (map[i][j] == 'M')
				{
					playmap_sprite.setPosition(j * 90, i * 90);
					window.draw(playmap_sprite);
				}

				if (map[i][j] == 'H')
				{
					hole_sprite.setPosition(j * 90, i * 90);
					window.draw(hole_sprite);
				}

				if (map[i][j] == 'W')
				{
					wizard_sprite.setPosition(j * 90, i * 90);
					window.draw(wizard_sprite);
				}

				if (map[i][j] == 'Y')
				{
					hydra_sprite.setPosition(j * 90, i * 90);
					window.draw(hydra_sprite);
				}

				if (map[i][j] == 'B')
				{
					bat_sprite.setPosition(j * 90, i * 90);
					window.draw(bat_sprite);
				}

				if (map[i][j] == 'E')
				{
					scientist_sprite.setPosition(j * 90, i * 90);
					window.draw(scientist_sprite);
				}

				if (map[i][j] == 'D')
				{
					diamond_sprite.setPosition(j * 90, i * 90);
					window.draw(diamond_sprite);
				}

				if (map[i][j] == 'r')
				{
					keyrelic_sprite.setPosition(j * 90, i * 90);
					window.draw(keyrelic_sprite);
				}

				if (map[i][j] == 'P')
				{
					paper_sprite.setPosition(j * 90, i * 90);
					window.draw(paper_sprite);
				}

				if (map[i][j] == 's')
				{
					script_sprite.setPosition(j * 90, i * 90);
					window.draw(script_sprite);
				}

				if (map[i][j] == 'S')
				{
					spiral_sprite.setPosition(j * 90, i * 90);
					window.draw(spiral_sprite);
				}

				if (map[i][j] == 'b')
				{
					bag_sprite.setPosition(j * 90, i * 90);
					window.draw(bag_sprite);
				}

				if (map[i][j] == 'w')
				{
					wall_sprite.setPosition(j * 90, i * 90);
					window.draw(wall_sprite);
				}

			}
		}

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (visited[i][j] == false)
				{
					mist_sprite.setPosition(j * 90, i * 90);
					window.draw(mist_sprite);
				}
			}
		}

		character_sprite.setPosition(colofplayer * 90, rowofplayer * 90);
		window.draw(character_sprite);

		char direction;
		sf::Event movement;
		while (window.pollEvent(movement))
		{
			if (movement.type == sf::Event::Closed)
			{
				window.close();
			}
			if (movement.type == sf::Event::KeyPressed)
			{
				if (movement.key.code == sf::Keyboard::Up)
				{
					player_movements(map, rowofplayer, colofplayer, 'U');
				}

				else if (movement.key.code == sf::Keyboard::Down)
				{
					player_movements(map, rowofplayer, colofplayer, 'D');
				}

				else if (movement.key.code == sf::Keyboard::Left)
				{
					player_movements(map, rowofplayer, colofplayer, 'L');
				}

				else if (movement.key.code == sf::Keyboard::Right)
				{
					player_movements(map, rowofplayer, colofplayer, 'R');
				}


				already_visited(map, visited, rowofplayer, colofplayer);
				grid_tile = get_tile(map, rowofplayer, colofplayer);
				interaction_with_tile(grid_tile);

				character_sprite.setPosition(colofplayer * 90, rowofplayer * 90);
				window.draw(character_sprite);
			}
		}
		window.display();
	}
	return 0;
}
