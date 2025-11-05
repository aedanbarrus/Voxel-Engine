#pragma once
#ifndef TerrainGenerator
#define TerrainGenerator
#include <vector>
#include <glm/glm.hpp>
#include "Random.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>
class GridSquare
{
public:
	glm::ivec2 pos;
	glm::ivec2 cpos;
	int count;
	GridSquare() : pos(glm::ivec2(0)), cpos(glm::ivec2(-1)), count(0) {}
	GridSquare(glm::ivec2 pos, glm::ivec2 connection = glm::ivec2(-1)) : pos(pos), cpos(connection), count(0) {}
	GridSquare clone()
	{
		GridSquare clone = GridSquare(pos, cpos);
		return clone;
	}
};
class Grid
{
public:
	int width;
	int height;
	int maxheight;
	std::vector<std::vector<GridSquare*>> grid;
	Grid(int w, int h) : width(w), height(h)
	{
		grid = {};
		for (int i = 0; i < h; i++)
		{
			std::vector<GridSquare*> temp = {};
			for (int j = 0; j < w; j++)
			{
				temp.push_back(nullptr);
			}
			grid.push_back(temp);
		}
	}

	GridSquare* getValue(glm::ivec2 pos)
	{
		return grid[pos.y][pos.x];
	}

	void setSquare(glm::ivec2 pos, glm::ivec2 cpos)
	{
		GridSquare* square = new GridSquare(pos, cpos);
		grid[pos.y][pos.x] = square;
	}
	void calculateCount(glm::ivec2 pos, int count)
	{
		if(count>maxheight)
			maxheight = count;
		if(grid[pos.y][pos.x] == nullptr)
			return;
		if (grid[pos.y][pos.x]->count < count)
		{
			grid[pos.y][pos.x]->count = count;
			if (grid[pos.y][pos.x]->cpos != glm::ivec2(-1))
			{
				calculateCount(grid[pos.y][pos.x]->cpos, count + 1);
			}
		}
	}

	void calculateCounts()
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				calculateCount(glm::ivec2(j, i), 1);
			}
		}
	}

	void addSquare()
	{
		glm::ivec2 pos;
		while (true)
		{
			pos = glm::vec2(int(random() * width), int(random() * height));
			if (grid[pos.y][pos.x] == nullptr)
			{
				break;
			}
		}
		while (true)
		{
			int dir = int(random() * 8);
			glm::ivec2 newPos = pos;
			if (dir > 3) dir++;
			newPos += glm::vec2((dir % 3) - 1, (dir / 3) - 1);
			if (newPos.x >= 0 && newPos.x < width && newPos.y >= 0 && newPos.y < height)
			{
				if (grid[newPos.y][newPos.x] != nullptr)
				{
					setSquare(pos, glm::ivec2(newPos.x, newPos.y));
					return;
				}
				pos = newPos;
			}
		}
	}
	void upscale()
	{
		height *= 3;
		width *= 3;
		std::vector<std::vector<GridSquare*>> tempgrid = {};
		for (int i = 0; i < height; i++)
		{
			std::vector<GridSquare*> temp = {};
			for (int j = 0; j < width; j++)
			{
				temp.push_back(nullptr);
			}
			tempgrid.push_back(temp);
		}
		for (int h = 0; h < height / 3; h++)
		{
			for (int w = 0; w < width / 3; w++)
			{
				if (grid[h][w] != nullptr)
				{
					GridSquare* sqr = grid[h][w];
					tempgrid[h * 3][w * 3] = new GridSquare();
					glm::ivec2 pos = glm::ivec2(w * 3, h * 3);
					glm::ivec2 cpos = sqr->cpos * 3;
					tempgrid[h * 3][w * 3]->pos = pos;
					if (sqr->cpos == glm::ivec2(-1, -1))
						continue;
					glm::ivec2 dir = (cpos - pos) / 3;
					glm::ivec2 offset = glm::ivec2(0);
					glm::ivec2 imidpos = glm::vec2(-1);
					if (glm::length(glm::vec2(dir)) == 1)
					{
						offset = glm::ivec2(dir.y, dir.x) * int(random() * 3.0f - 1.0f);
						if ((offset + pos).x < 0 || (offset + pos).y < 0)
						{
							offset = glm::ivec2(0);
						}
					}
					if (grid[h + dir.y + offset.y][w + dir.x + offset.x] != nullptr && grid[h + dir.y + offset.y][w + dir.x + offset.x]->cpos == glm::ivec2(h, w))
					{
						offset = glm::ivec2(0);
					}
					glm::ivec2 i1pos = pos + dir + offset;
					glm::ivec2 i2pos = pos + dir * 2 + offset * int(random() * 2.0f);
					if (glm::length(glm::vec2(dir)) != 1 && int(random() * 2.0f) == 0)
					{
						int x = 1;
						int y = 1;
						if (int(random() * 2.0f) == 0)
						{
							x = 0;
						}
						else
						{
							y = 0;
						}
						imidpos = pos + dir + dir * glm::ivec2(x, y);
					}
					if (imidpos != glm::ivec2(-1))
					{
						tempgrid[i1pos.y][i1pos.x] = new GridSquare(i1pos, imidpos);
						tempgrid[imidpos.y][imidpos.x] = new GridSquare(imidpos, i2pos);
						tempgrid[i2pos.y][i2pos.x] = new GridSquare(i2pos, cpos);
					}
					else
					{
						tempgrid[i1pos.y][i1pos.x] = new GridSquare(i1pos, i2pos);
						tempgrid[i2pos.y][i2pos.x] = new GridSquare(i2pos, cpos);
					}
					tempgrid[h * 3][w * 3]->cpos = i1pos;
				}
			}
		}
		maxheight = maxheight * 3 - 2;
		grid = tempgrid;
	}
};
class Heightmap
{
public:
	std::vector<std::vector<float>> data;
	Heightmap() : data({}) {}
	Heightmap(int width, int height)
	{
		data = {};
		for (int i = 0; i < height; i++)
		{
			std::vector<float> temp = {};
			for (int j = 0; j < width; j++)
			{
				temp.push_back(0.0f);
			}
			data.push_back(temp);
		}
	}
	Heightmap(Grid& grid)
	{
		data = {};
		for (int i = 0; i < grid.height; i++)
		{
			std::vector<float> temp = {};
			for (int j = 0; j < grid.width; j++)
			{
				if (grid.grid[i][j] != nullptr)
					temp.push_back(1.0f-1.0f/(1.0f+float(grid.grid[i][j]->count)));
				else
					temp.push_back(0.0f);
			}
			data.push_back(temp);
		}
	}
	float getValue(int x, int y)
	{
		if(y<0 || y>=data.size() || x<0 || x>=data[0].size())
			return 0.0f;
		return data[y][x];
	}
	void upscale()
	{
		int oldheight = data.size();
		int oldwidth = data[0].size();
		int newheight = oldheight * 3;
		int newwidth = oldwidth * 3;
		std::vector<std::vector<float>> newdata = {};
		for (int i = 0; i < newheight; i++)
		{
			std::vector<float> temp = {};
			for (int j = 0; j < newwidth; j++)
			{
				temp.push_back(0.0f);
			}
			newdata.push_back(temp);
		}
		for (int h = 0; h < oldheight; ++h)
		{
			for (int w = 0; w < oldwidth; ++w)
			{
				newdata[h * 3][w * 3] = getValue(w, h) * 0.5625 + getValue(w - 1, h) * 0.1875 + getValue(w, h - 1) * 0.1875 + getValue(w - 1, h - 1) * 0.0625;
				newdata[h * 3][w * 3 + 1] = getValue(w, h) * 0.75 + getValue(w, h - 1) * 0.25;
				newdata[h * 3][w * 3 + 2] = getValue(w, h) * 0.5625 + getValue(w + 1, h) * 0.1875 + getValue(w, h - 1) * 0.1875 + getValue(w + 1, h - 1) * 0.0625;
				newdata[h * 3 + 1][w * 3] = getValue(w, h) * 0.75 + getValue(w - 1, h) * 0.25;
				newdata[h * 3 + 1][w * 3 + 1] = getValue(w, h);
				newdata[h * 3 + 1][w * 3 + 2] = getValue(w, h) * 0.75 + getValue(w + 1, h) * 0.25;
				newdata[h * 3 + 2][w * 3] = getValue(w, h) * 0.5625 + getValue(w - 1, h) * 0.1875 + getValue(w, h + 1) * 0.1875 + getValue(w - 1, h + 1) * 0.0625;
				newdata[h * 3 + 2][w * 3 + 1] = getValue(w, h) * 0.75 + getValue(w, h + 1) * 0.25;
				newdata[h * 3 + 2][w * 3 + 2] = getValue(w, h) * 0.5625 + getValue(w + 1, h) * 0.1875 + getValue(w, h + 1) * 0.1875 + getValue(w + 1, h + 1) * 0.0625;
			}
		}
		data = newdata;
	}
	float blur_i(int x, int y)
	{
		for (int dy = -1; dy < 2; dy++)
		{
			for (int dx = -1; dx < 2; dx++)
			{
				float scale = 0.0f;
				if (glm::length(glm::vec2(dy, dx)) > 1.0f)
				{
					scale = 0.0625f;
				}
				else if (glm::length(glm::vec2(dy, dx)) == 1.0f)
				{
					scale = 0.125f;
				}
				else
				{
					scale = 0.25f;
				}
				return getValue(x + dx, y + dy) * scale;
			}
		}
	}
	void blur()
	{
		int height = data.size();
		int width = data[0].size();
		std::vector<std::vector<float>> newdata = {};
		for (int i = 0; i < height; i++)
		{
			std::vector<float> temp = {};
			for (int j = 0; j < width; j++)
			{
				temp.push_back(blur_i(j,i));
			}
			newdata.push_back(temp);
		}
		data = newdata;
	}
};



#endif