// Mono Calibration Console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <Eigen/src/SVD/JacobiSVD.h>

using namespace std;
using namespace Eigen;

void Compute_Projection_Matrix(int point_count, float* world_points, float* image_points, float* A);

void Reconstruct(int point_count, float* test1, float* p1, float* w);

void initFiles(int point_count, const char* image_path, const char* world_path, float* C_1, float* w);

int main()
{
	/******************************************************************
	 calibration
	 *******************************************************************/
	const char* image = "image points.txt";
	const char* world = "world points.txt";
	int N = 19;
	float* c1 = new float[N * 2];
	float* w = new float[N * 3];
	initFiles(N, image, world, c1, w);

	float* P1 = new float[12];
	Compute_Projection_Matrix(N, w, c1, P1);	//computes projection matrix

	/*******************************************************************
	reconstruction
	********************************************************************/
	int test_count = 4;
	float* rw = new float[test_count * 3];
	float* t1 = new float[test_count * 2];

	ifstream test1;
	test1.open("test.txt");
	for (int i = 0; i < test_count; i++) {
		test1 >> t1[i * 2] >> t1[i * 2 + 1];
	}
	test1.close();

	Reconstruct(test_count, t1, P1, rw);	//recovers the 3d positions from camera and their projection matrices

	for (int i = 0; i < test_count; i++) {
		cout << rw[i * 3] << "     " << rw[i * 3 + 1] << "     " << rw[i * 3 + 2] << endl;
	}

	std::cin.get();
}

void Compute_Projection_Matrix(int point_count, float * world_points, float * image_points, float * A)
{
	MatrixXf D(point_count * 2, 11);
	VectorXf R(point_count * 2);
	float X, Y, Z, x, y;
	// D ve R matrislerinin doldurulması
	for (int i = 0; i < point_count; i++) {
		X = world_points[i * 3];
		Y = world_points[i * 3 + 1];
		Z = world_points[i * 3 + 2];
		x = image_points[i * 2];
		y = image_points[i * 2 + 1];

		// R'nin doldurulması
		R(i * 2) = x;
		R(i * 2 + 1) = y;

		float current_point_data[22] = { X,Y,Z,1,0,0,0,0,-X * x,-Y * x,-Z * x,
										 0,0,0,0,X,Y,Z,1,-X * y,-Y * y,-Z * y };
		
		//D'nin doldurulması
		for (int j = 0; j < 11; j++) {
			D(i * 2, j) = current_point_data[j];
			D(i * 2 + 1, j) = current_point_data[11 + j];
		}
	}
}

void Reconstruct(int point_count, float * test1, float * p1, float * w)
{
	MatrixXf A(2, 3);
	Vector2f B;
	float x1, y1;
	for (int i = 0; i < point_count; i++) {
		x1 = test1[i * 2];
		y1 = test1[i * 2 + 1];

		A << x1 * p1[8] - p1[0], x1*p1[9] - p1[1], x1*p1[10] - p1[2],
			y1*p1[8] - p1[4], y1*p1[9] - p1[5], y1*p1[10] - p1[6];
		B << p1[3] - x1 * p1[11],
			p1[7] - y1 * p1[11];

		Vector3f solution;
		JacobiSVD<MatrixXf> svd(A, ComputeFullU | ComputeFullV);
		solution = svd.solve(B);
		w[i * 3 + 0] = solution(0);
		w[i * 3 + 1] = solution(1);
		w[i * 3 + 2] = solution(2);
	}
}

void initFiles(int point_count, const char * image_path, const char * world_path, float * C_1, float * w)
{
	std::ifstream image;
	image.open(image_path);
	std::ifstream world;
	world.open(world_path);

	for (int i = 0; i < point_count; i++) {
		image >> C_1[i * 2 + 0] >> C_1[i * 2 + 1];
		world >> w[i * 3 + 0] >> w[i * 3 + 1] >> w[i * 3 + 2];
	}

	image.close();
	world.close();
}
