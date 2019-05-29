#pragma once
#ifndef DRAW_H
#define DRAW_H

#include <vector>
#include <math.h>


using namespace std;

class Point
{
public:
	float x, y;
	void setxy(float mx, float my) {
		x = mx;
		y = my;
	}
};


vector<Point> pointSet;

float delta = 0.001;

class Draw {
private:

	int x;

public:
	/*
	void add(Point point) {
		pointSet.push_back(point);
	}

	void decrease() {
		if (!pointSet.empty())
			pointSet.pop_back();
	}

	int getSize() {
		return pointSet.size();
	}

	Point getPoint(int i) {
		return pointSet[i];
	}

	vector<Point> getPointSet() {
		return pointSet;
	}*/

	void drawLine(const int& shaderProgram, float x1, float y1, float x2, float y2, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
		float vertice[12] = {
			x1, y1, 0, color.x,  color.y, color.z,
			x2, y2, 0, color.x,  color.y, color.z
		};

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertice), vertice, GL_STATIC_DRAW);

		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glPointSize(2.0f);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);

	}

	void drawPoint(const int& shaderProgram, float x, float y, float pointSize, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
		float vertice[6] = { x, y, 0, color.x,  color.y, color.z };

		unsigned int VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertice), vertice, GL_STATIC_DRAW);

		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glPointSize(pointSize);
		glDrawArrays(GL_POINTS, 0, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}



	void drawBezierCurve(const int& shaderProgram, ImVec4 color) {
		int size = pointSet.size();
		int n = size - 1;
		float* B = new float[size];
		for (int i = 0; i < size; i++) {
			int k = i, x = 1;
			float c = 1.0;
			if (k > n - k)
				k = n - k;
			for (int j = n; j > n - k; j--, x++)
				c *= (float)j / x;
			B[i] = c;
		}

		for (float t = 0.0; t < 1.0; t += delta) {
			float x = 0.0, y = 0.0, tmp;
			for (int i = 0; i <= n; i++) {
				tmp = B[i] * pow(t, i) * pow(1 - t, n - i);
				x += tmp * pointSet[i].x;
				y += tmp * pointSet[i].y;
			}
			drawPoint(shaderProgram, x, y, 2.0f, color);
		}
	}



	void drawTangent(const int& shaderProgram, vector<Point> points, float t, ImVec4 color)
	{
		vector<Point> new_points;
		float x1, y1, x2, y2;
		Point point;

		while (points.size() >= 2) {
			new_points.clear();
			x2 = points[0].x + t * (points[1].x - points[0].x);
			y2 = points[0].y + t * (points[1].y - points[0].y);
			point.setxy(x2, y2);
			new_points.push_back(point);
			drawPoint(shaderProgram, x2, y2, 2.0f, color);

			for (int i = 2; i <= points.size() - 1; i++) {
				x1 = x2;
				y1 = y2;
				x2 = points[i - 1].x + t * (points[i].x - points[i - 1].x);
				y2 = points[i - 1].y + t * (points[i].y - points[i - 1].y);
				point.setxy(x2, y2);
				new_points.push_back(point);
				drawPoint(shaderProgram, x2, y2, 2.0f, color);
				drawLine(shaderProgram, x1, y1, x2, y2, color);
			}
			points.assign(new_points.begin(), new_points.end());
		}
		return;
	}

};

#endif